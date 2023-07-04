//
// Created by xyfuture on 2023/4/29.
//

#include "BaseCoreModule.h"



BaseCoreModule::BaseCoreModule(const sc_module_name &name, const CoreConfig &config,const SimConfig& sim_config, Core *core_ptr,ClockDomain* clk)
: sc_module(name), core_config(config), sim_config(sim_config), core_ptr(core_ptr),clk_ptr(clk) {

}

void BaseCoreModule::setEndPC(int pc) {
    end_pc = pc;
}

int BaseCoreModule::getEndPC() {
    return end_pc;
}

bool BaseCoreModule::isEndPC(int pc) {
    return pc == end_pc;
}

EnergyCounter BaseCoreModule::getEnergyCounter() {
    return energy_counter;
}
