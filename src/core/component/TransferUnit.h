//
// Created by xyfuture on 2023/3/30.
//



#ifndef CORE_COMPONENT_TRANSFERUNIT_H_
#define CORE_COMPONENT_TRANSFERUNIT_H_

#include <systemc>

#include "core/BaseCoreModule.h"
#include "comm/InitiatorSocket.h"
#include "core/payloads/StagePayloads.hpp"
#include "network/SwitchSocket.h"
#include "utils/Register.hpp"
#include "core/payloads/SynchroPayloads.hpp"
#include "utils/FSM.hpp"
#include "core/payloads/ExecInfo.h"
#include "utils/PulseSignal.h"

using namespace sc_core;

// TransferUnit
// Transfer Data between cores (send/recv) and off-chip memory and
// Synchronization between cores (wait/sync)
class TransferUnit: public BaseCoreModule{
    SC_HAS_PROCESS(TransferUnit);
public:
    TransferUnit(const sc_module_name& name,const CoreConfig& config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk,int core_id);

    void me_checkTransferInst();

    void process();

    void processSendInst(const TransferInfo& transfer_info);
    void processRecvInst(const TransferInfo& transfer_info);

    void processSyncInst(const TransferInfo& transfer_info);
    void processWaitInst(const TransferInfo& transfer_info);

    void processGlobalMemoryInst(const TransferInfo& transfer_info);

    void switchReceiveHandler(std::shared_ptr<NetworkPayload> trans,sc_time& delay);


    std::string getStatus();

private:
    int core_id;

private:
    sc_event switch_finish_trigger;

private:
    // wait sync
    int event_register[32]{};
    sc_event sync_wait_trigger;

    int wait_ev_addr = -1; // -1 for not waiting
    int wait_ev_value = -1;

private:
    // send recv
    sc_event send_trigger; // wait under send inst
    sc_event recv_trigger; // wait under recv inst

    // -1 for not ready
    int send_receiver_core_id = -1;
    int recv_sender_core_id = -1; // execute recv and recv sender_core_id's send inst

    std::map<int,int> recv_waiting_sender_map; // <core_id,wait_inst_tag>
    int send_inst_tag = -1; // use pc as tag to identify two send inst

private:
    FSM<ExecInfo> transfer_fsm_reg;
    sc_signal<VP<ExecInfo>> transfer_fsm_in;

    PulsePort<ExecInfo> pulse_commit;

public:
    sc_in<VP<ExecInfo>> transfer_port;
    sc_out<bool> transfer_ready_port;

    InitiatorSocket memory_socket;
    SwitchSocket switch_socket;

    sc_out<ExecInfo> transfer_commit_port;


};


#endif //CORE_COMPONENT_TRANSFERUNIT_H_
