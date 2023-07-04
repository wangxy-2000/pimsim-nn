//
// Created by xyfuture on 2023/4/12.
//

#include "ExternalMemory.h"

ExternalMemory::ExternalMemory(const std::string &name, const MemoryConfig &config, int switch_id)
:name(name), mem("memory",config), wrapper("wrapper"), mem_switch("switch",switch_id)
{
    wrapper.initiator_socket.bind(&mem.target_socket);
    wrapper.switch_socket.bind(&mem_switch);
}

EnergyCounter ExternalMemory::getEnergyCounter() {
    return mem.getEnergyCounter();
}
