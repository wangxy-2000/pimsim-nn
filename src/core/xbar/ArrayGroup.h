//
// Created by xyfuture on 2023/8/17.
//



#ifndef CORE_XBAR_ARRAYGROUP_H_
#define CORE_XBAR_ARRAYGROUP_H_

#include <systemc>
#include "core/xbar/XbarArray.h"
#include "core/payloads/ExecInfo.h"
#include "utils/Register.hpp"
#include "utils/ValidPayload.h"
#include "utils/FSM.hpp"
#include "utils/EnergyCounter.h"
#include "comm/InitiatorSocket.h"

using namespace sc_core;

class AGDispatcher:public sc_module{
    SC_HAS_PROCESS(AGDispatcher);
public:
    AGDispatcher(const sc_module_name& name, int out_n_, ClockDomain* clk);

    void me_dispatch();

private:
    RegEnable<VP<ExecInfo>> reg;
    sc_signal<VP<ExecInfo>> reg_out;
    sc_signal<bool> reg_enable;

    int out_n;

public:
    sc_in<VP<ExecInfo>> data_in;
    sc_out<bool> in_ready;

    std::vector<sc_out<VP<ExecInfo>>> data_out_array;
    std::vector<sc_in<bool>> out_ready_array;
};

class CommitQueue:public sc_module{
    SC_HAS_PROCESS(CommitQueue);

public:
    explicit CommitQueue(const sc_module_name& name,ClockDomain* clk);

    void me_processCommit();

    void commit(const ExecInfo& exec_info);

private:
    std::queue<ExecInfo> q;
    sc_event trigger;
    ClockDomain* clk;

public:
    sc_out<ExecInfo> out_port;
};

class ArrayGroup: public sc_module{
    SC_HAS_PROCESS(ArrayGroup);
public:
    ArrayGroup(const sc_module_name& name,const MatrixUnitConfig& matrix_config_,ClockDomain* clk_,
               int xbar_count,int array_group_id,EnergyCounter* counter,CommitQueue* commit_queue);

    void th_processMVM();

    int getArrayGroupComputeLatencyEnergy(const MatrixInfo& matrix_info);


    std::string getStatus();

protected:
    void start_of_simulation() override;


private:
    XbarArray xbar_array;
    int xbar_count;
    int array_group_id;

    EnergyCounter* energy_counter;
    const MatrixUnitConfig& matrix_config;
    CommitQueue* commit_queue;

private:
    FSM<ExecInfo> fsm;


public:
    sc_out<bool> in_ready;
    sc_in<VP<ExecInfo>> fsm_in;
    InitiatorSocket memory_socket;

};


#endif //CORE_XBAR_ARRAYGROUP_H_
