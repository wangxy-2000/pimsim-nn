//
// Created by xyfuture on 2023/3/17.
//

#include <functional>
#include "Memory.h"


Memory::Memory(const sc_module_name &name, const MemoryConfig &config)
:sc_module(name),config(config), target_socket("target_socket"){
    target_socket.registerHandler(
                [this](TransactionPayload& trans,sc_time& delay){
                    this->transport(trans,delay);
                }
            );

    energy_counter.setStaticPowerMW(config.static_power);
}

// Process Memory Request
void Memory::transport(TransactionPayload & trans, sc_time & delay) {
    const auto& current_time = sc_time_stamp();

    auto _latency_cycle = getTransLatencyCyclePower(trans);
    auto _latency = _latency_cycle * config.period;

    auto latency = sc_time(_latency,sc_core::SC_NS);
    if (current_time > latest_request_finish_time){
        delay += latency;
        latest_request_finish_time = current_time + latency;
    }
    else{
        delay = latest_request_finish_time + latency - current_time;
        latest_request_finish_time = latest_request_finish_time + latency;
    }

}

int Memory::getTransLatencyCyclePower(TransactionPayload & trans) {
    auto size = trans.data_size;
    auto times = int(ceil(size*1.0/config.data_width));

    if (trans.command == +TransCommand::read) {
        auto read_energy = config.read_dynamic_energy* times;

        energy_counter.addDynamicEnergyPJ(read_energy);
        return times * config.read_latency_cycle;
    }
    else if (trans.command == +TransCommand::write){
        auto write_energy = config.write_dynamic_energy * times;

        energy_counter.addDynamicEnergyPJ(write_energy);
        return times*config.write_latency_cycle;
    }

    assert(false);
    return 0;
}

EnergyCounter Memory::getEnergyCounter() {
    return energy_counter;
}
