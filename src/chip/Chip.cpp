//
// Created by xyfuture on 2023/4/10.
//
#include "Chip.h"
#include <sstream>
#include <fmt/core.h>

Chip::Chip(const ChipConfig &chip_config_,const SimConfig& sim_config_)
: global_memory("global_memory", chip_config_.global_memory_config, chip_config_.global_memory_switch_id),
  network("global_network", chip_config_.network_config),
  chip_config(chip_config_), sim_config(sim_config_),
  clk("ClockDomain",chip_config_.core_config.period){
    auto core_cnt = chip_config.core_cnt;
    // initialize cores
    for (int i=0;i<core_cnt;i++){
        core_array.push_back(std::make_shared<Core>(
                (std::string("core")+std::to_string(i)).c_str(),chip_config_.core_config,sim_config_,i,this,&clk)
            );
    }

    // connect global memory and network
    global_memory.mem_switch.bind(&network);

    // connect core and network
    for (const auto& core : core_array){
        core->switchBind(&network);
    }

}

void Chip::setEnergyCounter() {
    energy_counter.initialize(); // set to zero

    for (auto& core:core_array){
        energy_counter += core->getEnergyCounter();
    }

    energy_counter += network.getEnergyCounter();
    energy_counter += global_memory.getEnergyCounter();


    energy_counter.setRunningTimeNS(sc_time_stamp().to_seconds()*1e9);
}

void Chip::setInstBuffer(const std::vector<std::vector<Instruction>> &inst_buffers) {
    assert(inst_buffers.size() == core_array.size());
    for (int i=0;i<chip_config.core_cnt;i++){
        core_array[i]->setInstBuffer(inst_buffers[i]);
    }
}

void Chip::readInstFromJson(const nlohmann::json &json_inst) {
    assert(json_inst.size() == core_array.size());
    for (int i=0;i<chip_config.core_cnt;i++){
        auto core_key = std::string ("core")+std::to_string(i);
        const auto& core_json_inst = json_inst.at(core_key);
        core_array[i]->readInstFromJson(core_json_inst);
    }
}

bool Chip::isFinish() {
    for (auto& core_ptr:core_array){
        if (!core_ptr->isFinish())
            return false;
    }
    return true;
}


std::string Chip::getSimulationReport() {
    // 输出模拟的结果

    setEnergyCounter();
    std::stringstream  s;

    s<<"Chip Simulation Result:\n";



    if (sim_config.sim_mode == 0){
        // time mode
        s<<fmt::format("  - {:<20}{:.3} samples\n","output count:",getRunRounds());
        s<<fmt::format("  - {:<20}{:.3} samples/s\n","throughput:",(getRunRounds()/(sim_config.sim_time/1e3)));
        s<<fmt::format("  - {:<20}{:.3} ms\n","average latency:",(sim_config.sim_time/getRunRounds()));
        s<<fmt::format("  - {:<20}{:.3} mW\n","average power:",(energy_counter.getAveragePowerMW()));
        s<<fmt::format("  - {:<20}{:.3} pJ/it\n","average energy:",(energy_counter.getTotalEnergyPJ()/getRunRounds()));
    }
    else if (sim_config.sim_mode == 1){
        sc_time finish_time(0,sc_core::SC_NS);
        for (const auto& core : core_array)
            if (core->getFinishTime()>finish_time)
                finish_time = core->getFinishTime();
        s<<fmt::format("  - {:<20}{:.2} ms\n","latency:",finish_time.to_seconds()*1000);
    }

    // more information
    if (sim_config.report_verbose_level >= 1){
        // core information
        for (const auto& core:core_array){
            s<<core->getSimulationReport();
        }
    }

    return s.str();
}

double Chip::getRunRounds() {
    double cnt=0;

    for (const auto& core_ptr:core_array){
        cnt += core_ptr->getRunRounds();
    }

    return cnt/chip_config.core_cnt;
}

