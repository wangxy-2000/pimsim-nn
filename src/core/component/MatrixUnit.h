//
// Created by xyfuture on 2023/3/30.
//



#ifndef CORE_COMPONENT_MATRIXUNIT_H_
#define CORE_COMPONENT_MATRIXUNIT_H_

#include <memory>
#include <systemc>

#include "core/BaseCoreModule.h"
#include "comm/InitiatorSocket.h"
#include "core/payloads/StagePayloads.hpp"
#include "utils/Register.hpp"
#include "core/xbar/ArrayGroup.h"
#include "utils/FSM.hpp"


using namespace sc_core;




// MatrixUnit: Matrix Execute Unit
// Matrix Unit mainly consists of many ReRAM array (or other processing-in-memory devices)
class MatrixUnit: public BaseCoreModule{
    SC_HAS_PROCESS(MatrixUnit);
public:
    MatrixUnit(const sc_module_name& name, const std::vector<int>& array_group_map_ , const CoreConfig& config, const SimConfig& sim_config, Core* core_ptr, ClockDomain* clk);

    void me_checkMatrixInst();

    void bindMemorySocket(TargetSocket* target_socket);

    std::string getStatus();

private:
    std::vector<int> xbar_group_map;

    AGDispatcher ag_dispatcher;
    std::vector<std::unique_ptr<ArrayGroup>> array_groups;
    CommitQueue commit_queue;

private:
    std::vector<sc_signal<bool>> ag_ready_array;

    sc_signal<VP<ExecInfo>> ag_dispatcher_in;
    std::vector<sc_signal<VP<ExecInfo>>> ag_dispatcher_out;



public:
    sc_in<VP<ExecInfo>> matrix_port;
    sc_out<bool> matrix_ready_port;
    sc_out<ExecInfo> matrix_commit_port;


};


#endif //CORE_COMPONENT_MATRIXUNIT_H_
