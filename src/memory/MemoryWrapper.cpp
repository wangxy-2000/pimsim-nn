//
// Created by xyfuture on 2023/3/24.
//

#include "MemoryWrapper.h"

#include <utility>

void MemoryWrapper::forward(std::shared_ptr<NetworkPayload> trans, sc_time &delay) {
    // change NetworkPayload to TransactionPayload
    // use initiator send to memory

    auto memory_trans = trans->getRequestPayload<TransactionPayload>();

    initiator_socket.transport(*memory_trans,delay);

    trans->response_payload = memory_trans;
}

MemoryWrapper::MemoryWrapper(std::string name):
name(std::move(name)), initiator_socket("memory_socket"), switch_socket("switch_socket"){
    auto tmp = std::bind(&MemoryWrapper::forward,this,std::placeholders::_1,std::placeholders::_2);
    switch_socket.registerReceiveHandler(tmp); // only support receive

}
