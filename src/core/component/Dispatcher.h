//
// Created by xyfuture on 2023/8/4.
//



#ifndef CORE_COMPONENT_DISPATCHER_H_
#define CORE_COMPONENT_DISPATCHER_H_


#include <systemc>
#include "core/component/ReorderBuffer.h"
#include "core/BaseCoreModule.h"
#include "utils/Register.hpp"



using namespace sc_core;


class Dispatcher: public BaseCoreModule{
    SC_HAS_PROCESS(Dispatcher);
public:
    Dispatcher(const sc_module_name& name,const CoreConfig& config,const SimConfig& simConfig,Core* core_ptr,ClockDomain* clk);

    void me_process();

    bool isDispatchable();

    std::string getStatus();

private:
    RegPipe<VP<ExecInfo>> dispatcher_reg;
    sc_signal<VP<ExecInfo>> dispatcher_reg_out;
    sc_signal<bool> self_ready;

public:
    sc_in<VP<ExecInfo>> id_dispatcher_port;
    sc_out<VP<ExecInfo>> dispatcher_out_port;

    sc_in<bool> rob_ready; // rob is ready

    sc_in<bool> matrix_ready_port;
    sc_in<bool> vector_ready_port;
    sc_in<bool> transfer_ready_port;

    sc_out<ExecInfo> pend_inst;
    sc_in<bool> is_pend_inst_conflict;

    sc_out<bool> dispatcher_ready_port;

};


#endif //CORE_COMPONENT_DISPATCHER_H_
