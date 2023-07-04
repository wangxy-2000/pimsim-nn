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


using namespace sc_core;


// VectorUnit: Vector Execute Unit
// Vector Unit is CMOS circuit and execute vector inst
class VectorUnit: public BaseCoreModule{
    SC_HAS_PROCESS(VectorUnit);
public:
    VectorUnit(const sc_module_name& name,const CoreConfig& config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk);

    void process();
    int getVectorComputeLatencyCyclePower(const VectorInfo& vector_info);

    void checkVectorInst();

    std::string getStatus();

private:
    static const std::set<Opcode> inst_read_double_vector;
    static const std::set<Opcode> inst_read_single_vector;

    static const std::set<Opcode> inst_use_output_byte;


private:
    // internal register
    FSM<VectorInfo> vector_fsm_reg;
    sc_signal<VectorInfo> vector_fsm_out;
    sc_signal<FSMPayload<VectorInfo>> vector_fsm_in;


public:
    sc_in<VectorInfo> id_vector_port;

    sc_out<bool> vector_busy_port;

    InitiatorSocket memory_socket ;

};


#endif //CORE_COMPONENT_VECTORUNIT_H_
