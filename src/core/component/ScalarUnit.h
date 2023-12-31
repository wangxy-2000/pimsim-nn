//
// Created by xyfuture on 2023/3/6.
//



#ifndef CORE_COMPONENT_SCALARUNIT_H_
#define CORE_COMPONENT_SCALARUNIT_H_

#include <systemc>

#include "core/BaseCoreModule.h"
#include "utils/Register.hpp"
#include "core/payloads/StagePayloads.hpp"


using namespace sc_core;


class ScalarUnit:public BaseCoreModule {
    SC_HAS_PROCESS(ScalarUnit);
public:
    ScalarUnit(const sc_module_name& name,const CoreConfig& config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk);

    void process();

    std::string getStatus();


private:
    RegPipe<VP<ScalarInfo>> scalar_reg;
//    RegNext<ScalarInfo> scalar_reg;

    sc_signal<VP<ScalarInfo>> scalar_reg_out;
    sc_signal<bool> self_ready;

public:
    sc_in<VP<ScalarInfo>> id_scalar_port;
    sc_out<RegFileWrite> reg_file_write_port;
    sc_out<bool> scalar_ready_port;

};


#endif //CORE_COMPONENT_SCALARUNIT_H_
