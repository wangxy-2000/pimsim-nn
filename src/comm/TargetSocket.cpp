//
// Created by xyfuture on 2023/3/21.
//

#include "TargetSocket.h"
#include "comm/InitiatorSocket.h"
#include <utility>

TargetSocket::TargetSocket(std::string name) :
socket_name(std::move(name)),handler(nullptr){}

void TargetSocket::bind(InitiatorSocket* initiator) {
    initiator->bind(this);
}

void TargetSocket::registerHandler(const std::function<void(TransactionPayload &, sc_time &)>& _handler) {
    handler = _handler;
}

void TargetSocket::Handler(TransactionPayload & trans, sc_time & delay) {
    if (handler) handler(trans,delay);
}

bool TargetSocket::checkHandler() {
    if (handler) return true;
    return false;
}





