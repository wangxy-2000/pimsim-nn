//
// Created by xyfuture on 2023/3/15.
//



#ifndef CORE_COMPONENT_VECTORUNIT_H_
#define CORE_COMPONENT_VECTORUNIT_H_

#include <systemc>
#include <set>
#include "core/BaseCoreModule.h"
#include "core/payloads/StagePayloads.hpp"
#include "utils/Register.hpp"
#include "comm/InitiatorSocket.h"
#include "utils/FSM.hpp"
#include "core/payloads/ExecInfo.h"
#include "utils/PulseSignal.h"


using namespace sc_core;


// VectorUnit: Vector Execute Unit
// Vector Unit is CMOS circuit and execute vector inst
class VectorUnit: public BaseCoreModule{
    SC_HAS_PROCESS(VectorUnit);
public:
    VectorUnit(const sc_module_name& name,const CoreConfig& config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk);

    void process();
    int getVectorComputeLatencyCyclePower(const VectorInfo& vector_info);

    void me_checkVectorInst();

    std::string getStatus();

private:
    static const std::set<Opcode> inst_read_double_vector;
    static const std::set<Opcode> inst_read_single_vector;

    static const std::set<Opcode> inst_use_output_byte;


private:
    // internal register
    FSM<ExecInfo> vector_fsm_reg;
    sc_signal<VP<ExecInfo>> vector_fsm_in;

    PulsePort<ExecInfo> pulse_commit;


public:
    sc_in<VP<ExecInfo>> vector_port;

    sc_out<bool> vector_ready_port;

    sc_out<ExecInfo> vector_commit_port;

    InitiatorSocket memory_socket;

};


#endif //CORE_COMPONENT_VECTORUNIT_H_
