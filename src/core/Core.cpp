//
// Created by xyfuture on 2023/3/30.
//

#include "Core.h"
#include "fmt/core.h"
#include <sstream>

Core::Core(const sc_module_name &name,const CoreConfig& core_config_,const SimConfig& sim_config_, int core_id,Chip* chip,ClockDomain* clk)
: sc_module(name), core_config(core_config_), sim_config(sim_config_), core_id(core_id), chip_ptr(chip),clk_ptr(clk),
  controller("controller",core_config_,sim_config_,this),
  inst_fetch("inst_fetch",core_config_,sim_config_,this,clk),
  inst_decode("inst_decode",core_config_,sim_config_,this,clk),
  reg_file("reg_file",core_config_,sim_config_,this,clk),
  scalar_unit("scalar_unit",core_config_,sim_config_,this,clk),
  matrix_unit("matrix_unit",core_config_,sim_config_,this,clk),
  vector_unit("vector_unit",core_config_,sim_config_,this,clk),
  transfer_unit("transfer_unit",core_config_,sim_config_,this,clk,core_id),
  core_switch("core_switch",core_id),
  local_memory("local_memory",core_config_.local_memory_config){

    inst_fetch.if_id_port.bind(fetch_decode);
    inst_decode.if_id_port.bind(fetch_decode);

    //Bind ports

    // decode info
    inst_decode.id_matrix_port.bind(decode_matrix);
    matrix_unit.id_matrix_port.bind(decode_matrix);

    inst_decode.id_vector_port.bind(decode_vector);
    vector_unit.id_vector_port.bind(decode_vector);

    inst_decode.id_transfer_port.bind(decode_transfer);
    transfer_unit.id_transfer_port.bind(decode_transfer);

    inst_decode.id_scalar_port.bind(decode_scalar);
    scalar_unit.id_scalar_port.bind(decode_scalar);

    // register field
    inst_decode.reg_file_read_addr_port.bind(decode_register);
    reg_file.reg_file_read_addr_port.bind(decode_register);

    inst_decode.reg_file_read_value_port.bind(register_decode);
    reg_file.reg_file_read_value_port.bind(register_decode);

    scalar_unit.reg_file_write_port.bind(scalar_register);
    reg_file.reg_file_write_port.bind(scalar_register);

    // busy info
    inst_decode.matrix_busy_port.bind(matrix_busy_decode);
    matrix_unit.matrix_busy_port.bind(matrix_busy_decode);

    inst_decode.vector_busy_port.bind(vector_busy_decode);
    vector_unit.vector_busy_port.bind(vector_busy_decode);

    inst_decode.transfer_busy_port.bind(transfer_busy_decode);
    transfer_unit.transfer_busy_port.bind(transfer_busy_decode);

    inst_decode.scalar_busy_port.bind(scalar_busy_decode);
    scalar_unit.scalar_busy_port.bind(scalar_busy_decode);

    // controller
    inst_fetch.if_stall.bind(fetch_stall_controller);
    inst_decode.id_stall.bind(decode_stall_controller);

    controller.if_stall.bind(fetch_stall_controller);
    controller.id_stall.bind(decode_stall_controller);

    inst_fetch.if_enable.bind(controller_enable_fetch);
    inst_decode.id_enable.bind(controller_enable_decode);

    controller.if_enable.bind(controller_enable_fetch);
    controller.id_enable.bind(controller_enable_decode);

    // Bind memory
    matrix_unit.memory_socket.bind(&local_memory.target_socket);
    vector_unit.memory_socket.bind(&local_memory.target_socket);
    transfer_unit.memory_socket.bind(&local_memory.target_socket);

    // Bind switch
    transfer_unit.switch_socket.bind(&core_switch);

}

bool Core::isFinish() const {
//    return inst_fetch.isFinish();
    return finish_running;
}

void Core::setCompoEndPC(int pc) {
    matrix_unit.setEndPC(pc);
    vector_unit.setEndPC(pc);
    transfer_unit.setEndPC(pc);
    scalar_unit.setEndPC(pc);
}

void Core::setInstBuffer(const std::vector<Instruction> &inst_buffer) {
    inst_fetch.setInstBuffer(inst_buffer);
    setCompoEndPC(inst_fetch.inst_buffer_size-1);
}

void Core::readInstFromJson(const nlohmann::json &json_inst) {
    inst_fetch.readInstFromJson(json_inst);
    setCompoEndPC(inst_fetch.inst_buffer_size-1);
}

void Core::switchBind(Network *network) {
    core_switch.bind(network);
}

int Core::getCoreID() const {
    return core_id;
}

void Core::setFinish() {
    finish_running =true;
    finish_time_stamp = sc_time_stamp();
}

sc_time Core::getFinishTime() {
    if (finish_running)
        return finish_time_stamp;
    return {0,SC_NS};
}

void Core::setEnergyCounter() {
    energy_counter.initialize(); // from zero
    energy_counter += controller.getEnergyCounter();
    energy_counter += inst_fetch.getEnergyCounter();
    energy_counter += inst_decode.getEnergyCounter();

    energy_counter += reg_file.getEnergyCounter();
    energy_counter += scalar_unit.getEnergyCounter();

    energy_counter += matrix_unit.getEnergyCounter();
    energy_counter += vector_unit.getEnergyCounter();
    energy_counter += transfer_unit.getEnergyCounter();

    energy_counter += local_memory.getEnergyCounter();

    // sim_mode will affect finish time
    if (sim_config.sim_mode == 0)
        energy_counter.setRunningTimeNS(sc_time_stamp());
    else if (sim_config.sim_mode == 1)
        energy_counter.setRunningTimeNS(getFinishTime());
}


EnergyCounter Core::getEnergyCounter() {
    setEnergyCounter();
    return energy_counter;
}

int Core::getRunRounds() const {
    return run_rounds;
}

void Core::addRunRounds() {
    run_rounds ++ ;
}

std::string Core::getSimulationReport() {

    // calculate all energy consumption
    setEnergyCounter();

    std::stringstream s;
    s<<fmt::format("Core {} Information\n",core_id);

    if (sim_config.sim_mode == 0){
        s<<fmt::format("  - {:<20}{} samples\n","rounds:",getRunRounds());
        s<<fmt::format("  - {:<20}{} mW\n","average power:",energy_counter.getAveragePowerMW());
    }
    else if (sim_config.sim_mode == 1){
        s<<fmt::format("  - {:<20}{:.5} ms\n","finish time:",getFinishTime().to_seconds()*1000);
    }


    if (sim_config.report_verbose_level >=2 ){
        auto percent = [this](BaseCoreModule& m)->double {return (m.getEnergyCounter().getTotalEnergyPJ()
                                                          /this->energy_counter.getTotalEnergyPJ())*100;};

        s<<"  Energy Breakdown Information:\n";
        s<<fmt::format("    - {<20}{:.2}%\n","Controller:",percent(controller));
        s<<fmt::format("    - {<20}{:.2}%\n","Inst Fetch:",percent(inst_fetch));
        s<<fmt::format("    - {<20}{:.2}%\n","Inst Decode:",percent(inst_decode));
        s<<fmt::format("    - {<20}{:.2}%\n","Scalar Unit:",percent(scalar_unit));
        s<<fmt::format("    - {<20}{:.2}%\n","Register File:",percent(reg_file));
        s<<fmt::format("    - {<20}{:.2}%\n","Matrix Unit:",percent(matrix_unit));
        s<<fmt::format("    - {<20}{:.2}%\n","Transfer Unit:",percent(transfer_unit));
        s<<fmt::format("    - {<20}{:.2}%\n","Vector Unit:",percent(vector_unit));
        s<<fmt::format("    - {<20}{:.2}%\n","Local Memory:",(local_memory.getEnergyCounter().getTotalEnergyPJ()
                                                        /energy_counter.getTotalEnergyPJ())*100);

    }


    return s.str();

}



