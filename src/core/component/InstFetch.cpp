//
// Created by xyfuture on 2023/2/28.
//

#include <sstream>

#include "InstFetch.h"
#include "core/Core.h"



InstFetch::InstFetch(const sc_module_name& name, const CoreConfig &config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk)
: BaseCoreModule(name,config,sim_config,core_ptr,clk),
pc_reg("pc_reg",clk){
    pc_reg.input.bind(pc_in);
    pc_reg.output.bind(pc_out);
    pc_reg.enable.bind(id_ready_port);

    pc_reg.setReset(0,pc_reset);


    SC_METHOD(me_processFetchUpdate);
    sensitive << pc_out ;

    energy_counter.setStaticPowerMW(config.inst_fetch_static_power);
}


void InstFetch::me_processFetchUpdate() {
    auto cur_pc = pc_out.read();

    if (cur_pc == inst_buffer_size){
        // finish running
        if (sim_config.sim_mode == 1) {
            // no more new inst
            if_id_port.write({DecodeInfo(), false});
            std::cout<<"core :"<<core_ptr->getCoreID()<< " reached time:"<<sc_time_stamp()<<std::endl;
            return;
        }

        // can not reach
        throw "Inst Fetch Error: Sim mode 1 Can not reach";
    }

//    std::cout<<getStatus()<<std::endl;
    pc_reset.write(false);
    if (sim_config.sim_mode == 0 and cur_pc == inst_buffer_size-1 ) {
        // the last inst
        // loop all instructions
        // statistic
        core_ptr->addRunRounds();
        pc_reset.write(true);

//        std::cout<<getStatus()<<std::endl;

    }


    DecodeInfo decode_info = {.pc=cur_pc,.inst=inst_buffer[cur_pc]};
    if_id_port.write({decode_info, true});

    auto next_pc = cur_pc + 1; // update pc
    pc_in.write(next_pc);


    energy_counter.addDynamicEnergyPJ(0,sc_time_stamp(),core_config.period,core_config.inst_fetch_dynamic_power);

}

void InstFetch::setInstBuffer(const std::vector<Instruction>& buffer) {
    inst_buffer = buffer;
    inst_buffer_size = (int)inst_buffer.size();
}


std::string InstFetch::getStatus() {
    auto cur_pc = pc_out.read();
    auto inst = inst_buffer[cur_pc];

    std::stringstream  s;

    s<<"Core:"<<core_ptr->getCoreID()<<" "<<"Fetch>"<<" time: "<<sc_time_stamp().to_string()<<"\n";
    s<<"pc:"<<cur_pc<<"  "<<inst<<"\n";

    return s.str();
}

bool InstFetch::isFinish() {
    // only work in sim mode 1
    if (pc_out.read() == inst_buffer_size)
        return true;
    return false;
}

void InstFetch::readInstFromJson(const nlohmann::json &json_inst) {
    for (auto& cur_inst:json_inst)
        inst_buffer.emplace_back(cur_inst);
    inst_buffer_size = (int)inst_buffer.size();
}
