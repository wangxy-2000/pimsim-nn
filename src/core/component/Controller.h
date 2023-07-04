//
// Created by xyfuture on 2023/3/4.
//



#ifndef CORE_COMPONENT_CONTROLLER_H_
#define CORE_COMPONENT_CONTROLLER_H_

#include <systemc>
#include "core/BaseCoreModule.h"
using namespace sc_core;

// Control enable wire of InstDecode and InstFetch
class Controller:public BaseCoreModule {
public:
    SC_HAS_PROCESS(Controller);

    Controller(const sc_module_name& name,const CoreConfig& config,const SimConfig& sim_config,Core* core_ptr);

    void process();

public:

    sc_in<bool> if_stall;
    sc_in<bool> id_stall;

    sc_out<bool> if_enable;
    sc_out<bool> id_enable;

};


#endif //CORE_COMPONENT_CONTROLLER_H_
