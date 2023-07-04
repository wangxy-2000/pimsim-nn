//
// Created by xyfuture on 2023/3/30.
//



#ifndef CORE_COMPONENT_MATRIXUNIT_H_
#define CORE_COMPONENT_MATRIXUNIT_H_

#include <systemc>

#include "core/BaseCoreModule.h"
#include "comm/InitiatorSocket.h"
#include "core/payloads/StagePayloads.hpp"
#include "utils/Register.hpp"
#include "core/xbar/XbarArray.h"
#include "utils/FSM.hpp"


using namespace sc_core;

// MatrixUnit: Matrix Execute Unit
// Matrix Unit mainly consists of many ReRAM array (or other processing-in-memory devices)
class MatrixUnit: public BaseCoreModule{
    SC_HAS_PROCESS(MatrixUnit);
public:
    MatrixUnit(const sc_module_name& name,const CoreConfig& config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk);

    void checkMatrixInst();

    void process();
    int getMatrixComputeLatencyCyclePower(const MatrixInfo& matrix_info);

    std::string getStatus();

private:
    FSM<MatrixInfo> matrix_fsm;
    sc_signal<MatrixInfo> matrix_fsm_out;
    sc_signal<FSMPayload<MatrixInfo>> matrix_fsm_in;

private:
    XbarArray xbar_array;

public:
    sc_in<MatrixInfo> id_matrix_port;
    sc_out<bool> matrix_busy_port;

    InitiatorSocket memory_socket;
};


#endif //CORE_COMPONENT_MATRIXUNIT_H_
