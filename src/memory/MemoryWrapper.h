//
// Created by xyfuture on 2023/3/24.
//



#ifndef MEMORY_MEMORYWRAPPER_H_
#define MEMORY_MEMORYWRAPPER_H_

#include "memory/Memory.h"
#include "comm/InitiatorSocket.h"
#include "network/SwitchSocket.h"


// Memory Wrapper
// Integrate Switch Socket
class MemoryWrapper {
public:
    explicit MemoryWrapper(std::string name);

    void forward(std::shared_ptr<NetworkPayload> trans,sc_time& delay);

public:
    InitiatorSocket initiator_socket;
    SwitchSocket switch_socket;

    std::string name;
};


#endif //MEMORY_MEMORYWRAPPER_H_
