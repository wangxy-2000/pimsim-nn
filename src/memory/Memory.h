//
// Created by xyfuture on 2023/3/17.
//



#ifndef MEMORY_MEMORY_H_
#define MEMORY_MEMORY_H_
#include <systemc>
#include <queue>

#include "config/Config.hpp"
#include "comm/TargetSocket.h"
#include "utils/EnergyCounter.h"


using namespace sc_core;

// Simple Memory Model
// Static Latency
// FIFO request process
class Memory:sc_module {
public:
    Memory(const sc_module_name& name ,const MemoryConfig& config);

    // Process Memory Request
    void transport(TransactionPayload&,sc_time&);

    int getTransLatencyCyclePower(TransactionPayload& trans); // 计算延迟等等

    EnergyCounter getEnergyCounter();

private:
    const MemoryConfig& config;

    // seperated time
    sc_time latest_request_finish_time;

public:
    TargetSocket target_socket;

private:
    EnergyCounter energy_counter;

};


#endif //MEMORY_MEMORY_H_
