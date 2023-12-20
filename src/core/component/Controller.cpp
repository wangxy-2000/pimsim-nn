//
// Created by xyfuture on 2023/3/4.
//

#include "Controller.h"
#include "core/Core.h"

Controller::Controller(const sc_module_name& name,const CoreConfig& config,const SimConfig& sim_config,Core* core_ptr)
: BaseCoreModule(name,config,sim_config,core_ptr) {

    SC_METHOD(process);
    sensitive << if_stall << id_stall<<dispatcher_stall;

    energy_counter.setStaticPowerMW(config.controller_static_power);\
}


void Controller::process() {

    auto if_status = if_stall.read();
    auto id_status = id_stall.read();
    auto dispatcher_status = dispatcher_stall.read();

    if (dispatcher_status){
        if_enable.write(false);
        id_enable.write(false);
        dispatcher_enable.write(false);
    }
    else if (id_status){
        if_enable.write(false);
        id_enable.write(false);
        dispatcher_enable.write(true);
    }
    else if (if_status){
        if_enable.write(false);
        id_enable.write(true);
        dispatcher_enable.write(true);
    }
    else{
        if_enable.write(true);
        id_enable.write(true);
        dispatcher_enable.write(true);
    }

    energy_counter.addDynamicEnergyPJ(0,sc_time_stamp(),core_config.period,core_config.controller_dynamic_power);
}