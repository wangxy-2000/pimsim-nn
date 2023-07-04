//
// Created by xyfuture on 2023/3/27.
//

#pragma once
#ifndef CORE_PAYLOADS_SYNCHROPAYLOADS_H_
#define CORE_PAYLOADS_SYNCHROPAYLOADS_H_

#include <memory>
#include "better-enums/enum.h"

BETTER_ENUM(SynchroType,uint8_t,data_transfer,sync);

BETTER_ENUM(DataTransferStatus,int,sender_ready,receiver_ready,receiver_not_ready,send_data);

// Synchro for all core-core synchronization
// sync is for inst sync

struct DataTransferInfo{
    int sender_id;
    int receiver_id;

    bool is_sender;
    DataTransferStatus status;

    int send_inst_tag; // use send inst pc

    int send_data_size;
    std::shared_ptr<void> data_ptr;

};


struct SyncInfo{
    int event_reg_addr;
};

struct SynchroInfo{ // just use in sender
    SynchroType type;
    std::shared_ptr<DataTransferInfo> data_transfer_info;
    std::shared_ptr<SyncInfo> sync_info;
};

#endif //CORE_PAYLOADS_SYNCHROPAYLOADS_H_
