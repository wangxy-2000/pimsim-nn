//
// Created by xyfuture on 2023/8/4.
//

#include "ReorderBuffer.h"
#include "core/Core.h"


ReorderBuffer::ReorderBuffer(const sc_module_name &name, int _capacity, const CoreConfig &config,
                             const SimConfig &simConfig, Core *core_ptr, ClockDomain *clk)
:BaseCoreModule(name, config, simConfig, core_ptr,clk),capacity(_capacity),ring_buffer(_capacity),
 perf_counter(core_ptr->getCoreID()){


    SC_METHOD(me_processNewInst);
    sensitive<<new_inst;

    SC_METHOD(me_processCommitInst);
    sensitive<<matrix_commit_inst<<vector_commit_inst<<transfer_commit_inst;

    SC_METHOD(me_updateROB);
    sensitive<<update_rob;

    SC_METHOD(me_processPendInst);
    sensitive<<pend_inst<<rob_changed;

    SC_METHOD(me_checkBufferIsReady);
    sensitive<<rob_changed;

}

bool ReorderBuffer::checkConflict(const ExecInfo &exec_info) {
    if(not exec_info.isValid())
        return  false;

    for (const auto& rob_info:ring_buffer){
        if (rob_info.second) {// valid
            if (exec_info.isConflict(rob_info.first)) return true;
        }
    }
    return false;
}

void ReorderBuffer::me_processPendInst() {
    const auto& pend_info = pend_inst.read();

    if ( pend_info.isValid() and checkConflict(pend_info)){
        // conflict
        is_conflict.write(true);
        return;
    }

    is_conflict.write(false);
}

void ReorderBuffer::me_checkBufferIsReady() {
    if (ring_buffer.full()){
        rob_ready.write(false);
    } else {
        rob_ready.write(true);
    }
}

void ReorderBuffer::me_processNewInst() {
    if(new_inst.read().valid)
        clk_ptr->notifyNextPosEdge(&update_rob);
}

void ReorderBuffer::me_processCommitInst() {
    clk_ptr->notifyNextPosEdge(&update_rob);
}

void ReorderBuffer::me_updateROB() {
    // just run once

    const auto& cur_info = new_inst.read();
    const auto& matrix_commit_info = matrix_commit_inst.read();
    const auto& vector_commit_info = vector_commit_inst.read();
    const auto& transfer_commit_info = transfer_commit_inst.read();

    std::vector<ExecInfo> commit_infos = {
            matrix_commit_info,vector_commit_info,transfer_commit_info
    };

    const auto& new_info = cur_info.payload;
    if(cur_info.valid and new_info.isValid()){
        ring_buffer.push(new_info);

        perf_counter.startRecord(cur_info.payload.inst_ptr);
    }


    for (const auto& cur_commit:commit_infos){
        if(cur_commit.isValid()){
            if (sim_config.sim_mode == 1){
                auto last_inst = ring_buffer.lastRetired(cur_commit);
                if (last_inst.first){
                    if (last_inst.second.pc == core_ptr->getMaxPC()){
                        core_ptr->setFinish();
                        std::cout<<"core: "<<core_ptr->getCoreID()<<" finish"<<std::endl;
                     }
                }

            } else
                ring_buffer.retire(cur_commit);

            perf_counter.finishRecord(cur_commit.inst_ptr);
        }
    }

    rob_changed.notify();


}

std::string ReorderBuffer::getStatus() {
    std::stringstream  s;

    s<<"Core:"<<core_ptr->getCoreID()<<" ROB> "<<"time: "<<sc_time_stamp()<<"\n";
    s<<"pend inst:"<<pend_inst.read();
    s<<"rob content:\n";
    for (const auto& rob_info:ring_buffer){
        s<<"valid tag: "<< rob_info.second<<" "<<rob_info.first;
    }
    return s.str();
}


