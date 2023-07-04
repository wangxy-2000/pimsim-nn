//
// Created by xyfuture on 2023/4/12.
//



#ifndef MEMORY_EXTERNALMEMORY_H_
#define MEMORY_EXTERNALMEMORY_H_

#include "memory/Memory.h"
#include "memory/MemoryWrapper.h"
#include "network/Switch.h"


// A module at core-level
// switch <-> wrapper <-> memory
class ExternalMemory {

public:
    ExternalMemory(const std::string& name,const MemoryConfig& config,int switch_id);

    EnergyCounter getEnergyCounter();

public:
    Memory mem;
    MemoryWrapper wrapper;
    Switch mem_switch;

private:
    std::string name;


};


#endif //MEMORY_EXTERNALMEMORY_H_
