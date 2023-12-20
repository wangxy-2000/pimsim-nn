//
// Created by xyfuture on 2023/4/29.
//

#include "BaseCoreModule.h"



BaseCoreModule::BaseCoreModule(const sc_module_name &name, const CoreConfig &config,const SimConfig& sim_config, Core *core_ptr,ClockDomain* clk)
: sc_module(name), core_config(config), sim_config(sim_config), core_ptr(core_ptr),clk_ptr(clk) {

}


EnergyCounter BaseCoreModule::getEnergyCounter() {
    return energy_counter;
}
