//
// Created by xyfuture on 2023/3/21.
//



#pragma once
#ifndef COMM_TRANSACTIONPAYLOAD_H_
#define COMM_TRANSACTIONPAYLOAD_H_

#include <systemc>
#include <memory>
#include <better-enums/enum.h>

BETTER_ENUM(TransStatus,int,success=1,error=2,processing=3);
BETTER_ENUM(TransCommand,int,read=1,write=2);


// like TLM generic payload
struct TransactionPayload {
    TransCommand command;
    long long addr; // 64bit
    int data_size;
    std::shared_ptr<void> data_ptr; // real date pointer

};




#endif //COMM_TRANSACTIONPAYLOAD_H_
