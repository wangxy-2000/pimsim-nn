//
// Created by xyfuture on 2023/3/30.
//

#include "TransferUnit.h"
#include "core/Core.h"
#include <sstream>



TransferUnit::TransferUnit(const sc_module_name &name, const CoreConfig &config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk,int core_id)
: BaseCoreModule(name,config,sim_config,core_ptr,clk), pulse_commit("pulse_commit",clk),
  core_id(core_id),
  memory_socket("memory_socket"), switch_socket("switch_socket",&switch_finish_trigger),
  transfer_fsm_reg("transfer_fsm", clk){

    transfer_fsm_reg.input.bind(transfer_fsm_in);
    transfer_fsm_reg.in_ready.bind(transfer_ready_port);

    pulse_commit.out_port.bind(transfer_commit_port);

    switch_socket.registerReceiveHandler(std::bind(
            &TransferUnit::switchReceiveHandler,this,std::placeholders::_1,std::placeholders::_2));

    memset(event_register,0,sizeof(event_register));

    SC_THREAD(process);

    SC_METHOD(me_checkTransferInst);
    sensitive << transfer_port;


    energy_counter.setStaticPowerMW(config.transfer_static_power);
}

void TransferUnit::me_checkTransferInst() {
    const auto& cur_info = transfer_port.read();
    if (cur_info.valid and cur_info.payload.exec_unit == +ExecType::Transfer)
        transfer_fsm_in.write(cur_info);
    else
        transfer_fsm_in.write({ExecInfo(),false});


//    auto transfer_info = transfer_port.read();
//
//    if (is_payload_valid(transfer_info))
//        transfer_fsm_in.write({transfer_info, true});
//    else
//        transfer_fsm_in.write({transfer_info, false});

}

void TransferUnit::process()  {
    while (true) {
        // wait for new transfer inst
        wait(transfer_fsm_reg.start_exec);


        // stall pipeline
        // fsm will auto set ready to false
        // transfer_busy_port.write(true);

        const auto& exec_info = transfer_fsm_reg.read();
        auto transfer_info = exec_info.getTransferInfo();
        const auto& op = transfer_info.op;

        switch (op) {
            case Opcode::send :
//                std::cout<<getStatus()<<std::endl;
                processSendInst(transfer_info);
                break;
            case Opcode::recv:
//                std::cout<<getStatus()<<std::endl;
                processRecvInst(transfer_info);
                break;
            case Opcode::sync:
                processSyncInst(transfer_info);
                break;
            case Opcode::wait:
                processWaitInst(transfer_info);
                break;
            case Opcode::nop:
                break;
            default:
                processGlobalMemoryInst(transfer_info);
        }


        // exec finish

        // restore pipe line
        // fsm will auto set ready to true
        // transfer_busy_port.write(false);

        // reset fsm
//        transfer_fsm_reg.finish_exec.notify(SC_ZERO_TIME);
        transfer_fsm_reg.finishExec([this,exec_info]{pulse_commit.write(exec_info);});


        // TODO dynamic energy
    }
}

void TransferUnit::switchReceiveHandler(std::shared_ptr<NetworkPayload> trans, sc_time &delay) {
     // synchro

     auto send_payload = trans->getRequestPayload<SynchroInfo>();

     if (send_payload->type == +SynchroType::data_transfer){
         // send recv inst
         auto data_transfer_info = send_payload->data_transfer_info;
         auto ano_is_sender = data_transfer_info->is_sender;

         if (ano_is_sender){
             // remote core execute send inst to this core
             auto sender_core_id = data_transfer_info->sender_id;
             if (sender_core_id == recv_sender_core_id){
                 // this core execute recv and match remote core send
                 auto status = data_transfer_info->status;
                 if (status == +DataTransferStatus::sender_ready){
                     // this core already execute recv and remote core execute send now
                     auto response = std::make_shared<DataTransferInfo>(DataTransferInfo{
                         .sender_id=sender_core_id,.receiver_id=core_id,.is_sender=false,
                         .status=DataTransferStatus::receiver_ready,
                         .send_inst_tag=data_transfer_info->send_inst_tag,
                         .send_data_size=0,.data_ptr=nullptr
                     });

                     auto network_payload = std::make_shared<NetworkPayload>(NetworkPayload{
                             .src=core_id, .dst=sender_core_id, .request_data_size=1,
                             .request_payload=std::make_shared<SynchroInfo>(SynchroInfo{
                                     .type=SynchroType::data_transfer, .data_transfer_info=response, .sync_info=nullptr
                             }),
                             .response_data_size=0, .response_payload=nullptr
                     });

                     switch_socket.send(network_payload);

                     delay += sc_time(1,SC_NS);
                     return;
                 }
                 else if (status == +DataTransferStatus::send_data){
                     recv_trigger.notify();
                     return;
                 }

             }
             else{
                 // remote core send and this core recv not match

//                 auto response = std::make_shared<DataTransferInfo>(DataTransferInfo{
//                     .sender_id=sender_core_id,.receiver_id=core_id,.is_sender=false,
//                     .status=DataTransferStatus::receiver_not_ready,
//                     .send_data_size=0,.data_ptr=nullptr
//                 });
//
//                 trans->response_payload = response;
//                 trans->response_data_size = 1;
//                 delay += sc_time(1,SC_NS);

                 // record info
                 delay += sc_time(1,SC_NS);
                 recv_waiting_sender_map[sender_core_id]=data_transfer_info->send_inst_tag;

                 return;
             }

         }
         else {
             // remote core recv this core

             auto receiver_core_id = data_transfer_info->receiver_id;

             if (receiver_core_id == send_receiver_core_id){
                 // this core execute send and match remote core recv
                 if (data_transfer_info->send_inst_tag == send_inst_tag
                        and data_transfer_info->status == +DataTransferStatus::receiver_ready)
                    send_trigger.notify();
                 else // should not run to here
                     throw "TransferUnit: send tag not match";

                 return;
             }
             else{
                 // not match remote core recv ready
                 throw "TransferUnit: send-recv not match";
             }

         }

     }
     else if (send_payload->type == +SynchroType::sync){
         // sync wait
        auto sync_info = send_payload->sync_info;
        event_register[sync_info->event_reg_addr] += 1;
        if (sync_info->event_reg_addr == wait_ev_addr){
            if (event_register[wait_ev_addr] == wait_ev_value)
                sync_wait_trigger.notify();
        }

        return;
     }

}

void TransferUnit::processSendInst(const TransferInfo &transfer_info) {
    // first sender send request and wait for receiver's permission then read local memory and transfer data

    auto receiver_core_id = transfer_info.core;

    // set basic info
    send_receiver_core_id = receiver_core_id;
    send_inst_tag = transfer_info.pc;

    auto request = std::make_shared<DataTransferInfo>(DataTransferInfo{
        .sender_id=core_id,.receiver_id=receiver_core_id,.is_sender=true,
        .status=DataTransferStatus::sender_ready,
        .send_inst_tag = send_inst_tag,
        .send_data_size=0,.data_ptr=nullptr
    });

    auto network_payload = std::make_shared<NetworkPayload>(NetworkPayload{
        .src=core_id,.dst=receiver_core_id,
        .request_data_size=1,
        .request_payload=std::make_shared<SynchroInfo>(SynchroInfo{
            .type=SynchroType::data_transfer,.data_transfer_info=request,.sync_info=nullptr}),
        .response_data_size=1,.response_payload=nullptr
    });

    switch_socket.send(network_payload);

    // wait receiver ready and permit this send request
    wait(send_trigger);

    // receiver ready

    //read memory for local memory
    sc_time delay(0,sc_core::SC_NS);
    TransactionPayload trans = {.command=TransCommand::read,.addr=transfer_info.rd_value,
                                .data_size=transfer_info.size,.data_ptr=nullptr};
    memory_socket.transport(trans,delay);

    wait(delay);
    // read memory finish

    // send data to another core
    auto send_data_req = std::make_shared<DataTransferInfo>(DataTransferInfo{
        .sender_id=core_id,.receiver_id=receiver_core_id,.is_sender=true,
        .status=DataTransferStatus::send_data,
        .send_inst_tag = send_inst_tag,
        .send_data_size=transfer_info.size,.data_ptr=nullptr
    });

    auto send_data_network_payload = std::make_shared<NetworkPayload>(NetworkPayload{
        .src=core_id,.dst=receiver_core_id,.request_data_size=transfer_info.size,
        .request_payload = std::make_shared<SynchroInfo>(SynchroInfo{
            .type=SynchroType::data_transfer,.data_transfer_info=send_data_req,.sync_info=nullptr}),
        .response_data_size=0,.response_payload=nullptr
    });

    switch_socket.send(send_data_network_payload);


    // send finish
    send_receiver_core_id = -1;
}

void TransferUnit::processRecvInst(const TransferInfo &transfer_info) {
    auto sender_core_id = transfer_info.core;
    // set basic info
    recv_sender_core_id = sender_core_id;


    if (recv_waiting_sender_map.count(sender_core_id) == 1
        and recv_waiting_sender_map[sender_core_id] != -1) {
        // sender already ready and receiver notify self ready
        auto request = std::make_shared<DataTransferInfo>(DataTransferInfo{
                .sender_id=sender_core_id, .receiver_id=core_id, .is_sender=false,
                .status=DataTransferStatus::receiver_ready,
                .send_inst_tag=recv_waiting_sender_map[sender_core_id],
                .send_data_size=0, .data_ptr=nullptr
        });

        auto network_payload = std::make_shared<NetworkPayload>(NetworkPayload{
                .src=core_id, .dst=sender_core_id, .request_data_size=1,
                .request_payload=std::make_shared<SynchroInfo>(SynchroInfo{
                        .type=SynchroType::data_transfer, .data_transfer_info=request, .sync_info=nullptr
                }),
                .response_data_size=0, .response_payload=nullptr
        });

        recv_waiting_sender_map[sender_core_id] = -1; // disable

        switch_socket.send(network_payload);

    }
    // wait for sender core ready and send data to this core
    wait(recv_trigger);

    // for receive handler
    recv_sender_core_id = -1;

    // receive data and transfer to local memory
    sc_time delay(0,SC_NS);
    TransactionPayload trans = TransactionPayload{
        .command=TransCommand::write,.addr=transfer_info.rd_value,
        .data_size=transfer_info.size,.data_ptr=nullptr
    };
    memory_socket.transport(trans,delay);
    wait(delay);

    // finish recv
}

void TransferUnit::processSyncInst(const TransferInfo &transfer_info) {
    // send sync info to another core
    auto event_register_addr = transfer_info.event_register;
    auto dst_core = transfer_info.core;

    auto request = std::make_shared<SyncInfo>(SyncInfo{event_register_addr});

    auto network_payload = std::make_shared<NetworkPayload>(NetworkPayload{
        .src=core_id,.dst=dst_core,.request_data_size=1,
        .request_payload=std::make_shared<SynchroInfo>(SynchroInfo{
            .type=SynchroType::sync,.data_transfer_info=nullptr,.sync_info=request
        }),
        .response_data_size=1,.response_payload=nullptr
    });

    switch_socket.send(network_payload);

    // execute time, may vary
    wait(core_config.period, sc_core::SC_NS);
}

void TransferUnit::processWaitInst(const TransferInfo &transfer_info) {
    auto event_register_addr = transfer_info.event_register;
    auto wait_value = transfer_info.wait_value;

    wait_ev_addr = event_register_addr;
    wait_ev_value = wait_value;

    if (event_register[wait_ev_addr] != wait_ev_value){
        // not equal wait until satisfy
        wait(sync_wait_trigger);
    }
    // match condition

    // finished
    event_register[wait_ev_addr] = 0; //reset
    wait_ev_addr = -1;
    wait_ev_value = -1;
}

void TransferUnit::processGlobalMemoryInst(const TransferInfo &transfer_info) {
    auto op = transfer_info.op;
    // ld st lldi lmv

    if (op == +Opcode::ld){
        auto global_addr = transfer_info.rs1_value_double;
        auto size = transfer_info.size;

        auto local_addr = transfer_info.rd_value;

        auto global_trans = std::make_shared<TransactionPayload>(TransactionPayload{
            .command=TransCommand::read,.addr=global_addr,.data_size=size,.data_ptr=nullptr
        });

        auto network_payload = std::make_shared<NetworkPayload>(NetworkPayload{
            .src=core_id,.dst=core_config.global_memory_switch_id,// TODO memory addr
            .request_data_size=1,.request_payload=global_trans,
            .response_data_size=0,.response_payload=nullptr
        });

        switch_socket.transport(network_payload);
        wait(switch_finish_trigger); // wait for finish

        // write to local memory
        auto local_trans =TransactionPayload{
            .command=TransCommand::write,.addr=local_addr,.data_size=size,.data_ptr= nullptr
        };

        sc_time delay (0,sc_core::SC_NS);
        memory_socket.transport(local_trans,delay);
        wait(delay);

        // load global to local memory finish
    }
    else if (op == +Opcode::st){
        auto global_addr = transfer_info.rd_value_double;
        auto local_addr = transfer_info.rs1_value;

        auto size = transfer_info.size;

        auto local_trans = TransactionPayload{
            .command=TransCommand::read,.addr=local_addr,.data_size=size,.data_ptr=nullptr
        };

        sc_time delay (0,SC_NS);
        memory_socket.transport(local_trans,delay);
        wait(delay);

        auto global_trans = std::make_shared<TransactionPayload>(TransactionPayload{
            .command=TransCommand::write,.addr=global_addr,.data_size=size,.data_ptr=nullptr
        });

        auto network_payload = std::make_shared<NetworkPayload>(NetworkPayload{
            .src=core_id,.dst=core_config.global_memory_switch_id,// TODO memory addr
            .request_data_size=size,.request_payload=global_trans,
            .response_data_size=0,.response_payload=nullptr
        });

        switch_socket.transport(network_payload);
        wait(switch_finish_trigger);

        // finish store;

    }
    else if (op == +Opcode::lldi){ // local memory
        auto addr = transfer_info.rd_value;
        auto imm = transfer_info.imm;
        auto size = transfer_info.len;

        auto trans = TransactionPayload{
            .command=TransCommand::write,.addr=addr,.data_size=size,.data_ptr=nullptr
        };

        sc_time delay(0,SC_NS);
        memory_socket.transport(trans,delay);
        wait(delay);

    }
    else if (op == +Opcode::lmv){
        // first read local memory then write to local memory
        // different from vmv, lmv done without stride
        auto read_addr = transfer_info.rs1_value;
        auto write_addr = transfer_info.rd_value;
        auto size = transfer_info.len;

        auto read_trans = TransactionPayload{
            .command=TransCommand::read,.addr=read_addr,.data_size=size,.data_ptr=nullptr
        };

        auto write_trans = TransactionPayload{
            .command=TransCommand::write,.addr=write_addr,.data_size=size,.data_ptr=nullptr
        };

        sc_time delay(0,SC_NS);
        memory_socket.transport(read_trans,delay);
        memory_socket.transport(write_trans,delay);

        wait(delay);
    }

}

std::string TransferUnit::getStatus() {
    std::stringstream  s;

    s<<"Core:"<<core_ptr->getCoreID()<<"Transfer> time: "<<sc_time_stamp()<<"\n";

    const auto& cur_info = transfer_fsm_reg.read();

    s<<cur_info<<"\n";

    return s.str();
}

