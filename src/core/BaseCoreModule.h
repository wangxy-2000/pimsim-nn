//
// Created by xyfuture on 2023/4/29.
//



#ifndef CORE_BASECOREMODULE_H_
#define CORE_BASECOREMODULE_H_

#include <systemc>
#include "config/Config.hpp"
#include "utils/EnergyCounter.h"
#include "utils/ClockDomain.h"

using namespace sc_core;

class Core;

// BaseCoreModule: base class of all core components
class BaseCoreModule:public sc_module {

public:
    BaseCoreModule(const sc_module_name& name,const CoreConfig& config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk= nullptr);

    void setEndPC(int pc);
    int getEndPC();

    bool isEndPC(int pc);

    EnergyCounter getEnergyCounter();

protected:
    Core* const core_ptr;
    ClockDomain* clk_ptr;
    const CoreConfig& core_config;
    const SimConfig& sim_config;

protected:
    int end_pc; // the last pc (sim_mode = 1)

protected:
    EnergyCounter energy_counter;

};


#endif //CORE_BASECOREMODULE_H_
