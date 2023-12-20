//
// Created by xyfuture on 2023/3/30.
//

#include "Core.h"
#include "fmt/core.h"
#include <sstream>

Core::Core(const sc_module_name &name, const CoreConfig& core_config_, const SimConfig& sim_config_,
           int core_id_, const std::vector<int>& array_group_map_, Chip* chip, ClockDomain* clk)
: sc_module(name), core_config(core_config_), sim_config(sim_config_), core_id(core_id_), chip_ptr(chip), clk_ptr(clk),
  array_group_map(array_group_map_),array_group_cnt(array_group_map_.size()),
  inst_fetch("inst_fetch",core_config_,sim_config_,this,clk),
  inst_decode("inst_decode",core_config_,sim_config_,this,clk),
  dispatcher("dispatcher",core_config_,sim_config_,this,clk),
  reorder_buffer("reorder_buffer",core_config_.rob_size,core_config_,sim_config_,this,clk),
  reg_file("reg_file",core_config_,sim_config_,this,clk),
  scalar_unit("scalar_unit",core_config_,sim_config_,this,clk),
  matrix_unit("matrix_unit",array_group_map_,core_config_,sim_config_,this,clk),
  vector_unit("vector_unit",core_config_,sim_config_,this,clk),
  transfer_unit("transfer_unit", core_config_, sim_config_, this, clk, core_id_),
  core_switch("core_switch", core_id_),
  local_memory("local_memory",core_config_.local_memory_config){


    // bind ports
    inst_fetch.if_id_port.bind(fetch_decode);
    inst_fetch.id_ready_port.bind(decode_fetch_ready);

    inst_decode.if_id_port.bind(fetch_decode);
    inst_decode.id_ready_port.bind(decode_fetch_ready);
    inst_decode.id_dispatch_port.bind(decode_dispatch);
    inst_decode.dispatcher_ready_port.bind(dispatch_decode_ready);
    inst_decode.id_scalar_port.bind(decode_scalar);
    inst_decode.scalar_ready_port.bind(scalar_decode_ready);

    inst_decode.reg_file_read_addr_port.bind(decode_register);
    inst_decode.reg_file_read_value_port.bind(register_decode);



    dispatcher.id_dispatcher_port.bind(decode_dispatch);
    dispatcher.dispatcher_ready_port.bind(dispatch_decode_ready);

    dispatcher.dispatcher_out_port.bind(dispatch_out);
    dispatcher.pend_inst.bind(dispatch_rob_pend);

    dispatcher.is_pend_inst_conflict.bind(rob_dispatch_is_conflict);
    dispatcher.rob_ready.bind(rob_dispatch_ready);
    dispatcher.matrix_ready_port.bind(matrix_dispatch_ready);
    dispatcher.vector_ready_port.bind(vector_dispatch_ready);
    dispatcher.transfer_ready_port.bind(transfer_dispatch_ready);

    reorder_buffer.new_inst.bind(dispatch_out);
    reorder_buffer.pend_inst.bind(dispatch_rob_pend);
    reorder_buffer.matrix_commit_inst.bind(matrix_rob_commit);
    reorder_buffer.vector_commit_inst.bind(vector_rob_commit);
    reorder_buffer.transfer_commit_inst.bind(transfer_rob_commit);

    reorder_buffer.is_conflict.bind(rob_dispatch_is_conflict);
    reorder_buffer.rob_ready.bind(rob_dispatch_ready);


    matrix_unit.matrix_port.bind(dispatch_out);
    matrix_unit.matrix_ready_port.bind(matrix_dispatch_ready);
    matrix_unit.matrix_commit_port.bind(matrix_rob_commit);

    vector_unit.vector_port.bind(dispatch_out);
    vector_unit.vector_ready_port.bind(vector_dispatch_ready);
    vector_unit.vector_commit_port.bind(vector_rob_commit);

    transfer_unit.transfer_port.bind(dispatch_out);
    transfer_unit.transfer_ready_port.bind(transfer_dispatch_ready);
    transfer_unit.transfer_commit_port.bind(transfer_rob_commit);

    scalar_unit.id_scalar_port.bind(decode_scalar);
    scalar_unit.scalar_ready_port.bind(scalar_decode_ready);
    scalar_unit.reg_file_write_port.bind(scalar_register);


    // register field
    reg_file.reg_file_read_addr_port.bind(decode_register);
    reg_file.reg_file_read_value_port.bind(register_decode);
    reg_file.reg_file_write_port.bind(scalar_register);


    // Bind memory
    matrix_unit.bindMemorySocket(&local_memory.target_socket);
    vector_unit.memory_socket.bind(&local_memory.target_socket);
    transfer_unit.memory_socket.bind(&local_memory.target_socket);


    // Bind switch
    transfer_unit.switch_socket.bind(&core_switch);

}


void Core::setInstBuffer(const std::vector<Instruction> &inst_buffer) {
    inst_fetch.setInstBuffer(inst_buffer);
    max_pc = inst_fetch.inst_buffer_size-1;
}


void Core::readInstFromJson(const nlohmann::json &json_inst) {
    inst_fetch.readInstFromJson(json_inst);
    max_pc = inst_fetch.inst_buffer_size-1;
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

bool Core::isFinish() const {
    return finish_running;
}

int Core::getMaxPC() const{
    return max_pc;
}


sc_time Core::getFinishTime() {
    if (finish_running)
        return finish_time_stamp;
    return {0,SC_NS};
}

void Core::setEnergyCounter() {
    energy_counter.initialize(); // from zero

    // TODO add energy counter for dispatcher rob


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

const SimConfig &Core::getSimConfig() const {
    return sim_config;
}

const CoreConfig &Core::getCoreConfig() const {
    return core_config;
}

const std::vector<int>& Core::getArrayGroupMap() const {
    return array_group_map;
}

int Core::getArrayGroupCount() const {
    return array_group_cnt;
}

void Core::start_of_simulation() {
    decode_fetch_ready.write(true);
    dispatch_decode_ready.write(true);

    scalar_decode_ready.write(true);

    vector_dispatch_ready.write(true);
    matrix_dispatch_ready.write(true);
    transfer_dispatch_ready.write(true);

    rob_dispatch_ready.write(true);


//    sc_module::start_of_simulation();
}

std::string Core::getStatus() {
    return std::string();
}



