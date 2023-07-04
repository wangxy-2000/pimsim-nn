//
// Created by xyfuture on 2023/3/21.
//

#include "InitiatorSocket.h"
#include "comm/TargetSocket.h"
#include <utility>

InitiatorSocket::InitiatorSocket(std::string name):
socket_name(std::move(name)),target_socket(nullptr){}


void InitiatorSocket::bind(TargetSocket *target) {
    target_socket= target;
}

void InitiatorSocket::transport(TransactionPayload &trans, sc_time &delay) {
    if (target_socket){
        target_socket->Handler(trans,delay);
    }
}



