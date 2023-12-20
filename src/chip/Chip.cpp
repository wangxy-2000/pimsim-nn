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

    global_memory.mem_switch.bind(&network);

    // initializeCores() will process all cores

}

void Chip::setEnergyCounter() {
    energy_counter.initialize(); // set to zero

    energy_counter.setRunningTimeNS(running_time.to_seconds() * 1e9);

    for (auto &core: core_array) {
        auto cur_energy_counter = core->getEnergyCounter();
        energy_counter += cur_energy_counter;
    }

    energy_counter += network.getEnergyCounter();
    energy_counter += global_memory.getEnergyCounter();


    if (sim_config.sim_mode == 1){ // latency mode
        energy_counter.setRunningTimeNS(running_time.to_seconds() * 1e9);
    }  else if (sim_config.sim_mode == 0)
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
    // chip-level simulation results
    setRunningTime();
    setEnergyCounter();
    std::stringstream  s;

    s<<"Chip Simulation Result:\n";


    if (sim_config.sim_mode == 0){
        // throughput mode
        s<<fmt::format("  - {:<20}{:.3} samples\n","output count:",getRunRounds());
        s<<fmt::format("  - {:<20}{:.3} samples/s\n","throughput:",(getRunRounds()/(sim_config.sim_time/1e3)));
        s<<fmt::format("  - {:<20}{:.10} ms\n","average latency:",(sim_config.sim_time/getRunRounds()));
        s<<fmt::format("  - {:<20}{:.10} mW\n","average power:",(energy_counter.getAveragePowerMW()));
        s<<fmt::format("  - {:<20}{:.10} pJ/it\n","average energy:",(energy_counter.getTotalEnergyPJ()/getRunRounds()));
    }
    else if (sim_config.sim_mode == 1){
        // latency mode
        s<<fmt::format("  - {:<20}{:.10} ms\n","latency:",running_time.to_seconds()*1000);
        s<<fmt::format("  - {:<20}{:.10} mW\n","average power:",(energy_counter.getAveragePowerMW()));
        s<<fmt::format("  - {:<20}{} pJ\n","average energy:",(energy_counter.getTotalEnergyPJ()));
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
    int cores = 0;
    for (const auto& core_ptr:core_array){
        cnt += core_ptr->getRunRounds();
        cores ++ ;
    }
    // average rounds
//    return cnt/chip_config.core_cnt;
    return cnt/cores;
}

void Chip::initializeCores(const nlohmann::json &json_inst) {
    auto core_cnt = json_inst.at("config").at("core_cnt").get<int>();
    auto array_group_map = json_inst.at("config").at("array_group_map");

    for (int i=0;i<core_cnt;i++){
        // initialize array groups and connect all cores
        auto core_name = std::string("core") + std::to_string(i);

        if (not json_inst.contains(core_name)){
            continue;
        }
        std::vector<int> core_array_group_map;

        if (array_group_map.contains(core_name))
            core_array_group_map = array_group_map.at(core_name).get<std::vector<int>>();
        auto core_ptr = std::make_shared<Core>(
                core_name.c_str(),chip_config.core_config,sim_config,i ,core_array_group_map,this,&clk);
        core_ptr->switchBind(&network);

        // read inst
//        if (json_inst.contains(core_name))
        core_ptr->readInstFromJson(json_inst.at(core_name));
        core_array.push_back(core_ptr);
    }

}

std::map<std::string, double> Chip::getChipWeightedTime() {
    std::map<std::string,double> chip_weighted_time;
    for (const auto& core : core_array){
        auto weighted_time= core->reorder_buffer.perf_counter.getWeightedStatistics();
        for(const auto& item:weighted_time){
            if (chip_weighted_time.count(item.first)){
                chip_weighted_time[item.first] += item.second;
            } else
                chip_weighted_time[item.first] = item.second;
        }
    }
    return chip_weighted_time;
}

void Chip::setRunningTime() {

    if (sim_config.sim_mode == 0){
        running_time = sc_time_stamp();
    } else if (sim_config.sim_mode == 1){
        // the last core time
        running_time = sc_time(0,SC_NS);

        for (const auto& core : core_array)
            if (core->getFinishTime()>running_time)
                running_time = core->getFinishTime();
    }


}

std::string Chip::getChipWeightedTimeReport() {
    std::stringstream  s;
    auto chip_weighted_time = getChipWeightedTime();
    s<<"Chip Weighted Time:\n";
    for(const auto& item : chip_weighted_time){
        s<<item.first<<" : "<<item.second<<"\n";
    }

    return s.str();
}


std::string Chip::getCoreWeightedTimeReport(){
    std::stringstream  s;

    s<<"Cores Weighted Time:\n";

    for (const auto& core_ptr:core_array){
        auto core_id = core_ptr->getCoreID();
        auto weighted_time = core_ptr->reorder_buffer.perf_counter.getWeightedStatistics();
        s<<"Core:"<<core_id<<std::endl;
        for (const auto& item:weighted_time){
            std::cout<<item.first<<" time: "<<item.second<<std::endl;
        }
        s<<std::endl;
    }

    return s.str();
}

