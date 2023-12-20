 //
// Created by xyfuture on 2023/3/18.
//

#include "Network.h"
#include <cmath>
#include <ghc/filesystem.hpp>
#include <fstream>


namespace fs = ghc::filesystem;

 Network::Network(const std::string &name, const BusConfig &config):
name(name), config(config){


}

int Network::transfer(int src, int dst, int data_size) {


    auto per_flit_latency = latency_map[{src,dst}]; // unit:ns
    auto per_flit_energy = energy_map[{src,dst}];
    int times = ceil(data_size*1.0/config.bus_width);

    // accumulate energy
    energy_counter.addDynamicEnergyPJ(times*per_flit_energy);

    return times * per_flit_latency;
}

Switch *Network::getSwitch(int switch_id) {
    return switch_map[switch_id];
}

void Network::registerSwitch(int switch_id, Switch *_switch) {
    assert(switch_map.find(switch_id) == switch_map.end());

    switch_map[switch_id] = _switch;
}

EnergyCounter Network::getEnergyCounter() {
    return energy_counter;
}

void Network::setLatencyEnergy(const nlohmann::json & latency_energy) {
    // set latency
    auto latency_array = latency_energy["latency"];
    for(const auto& src_array:latency_array.items()){
        auto src_key = std::stoi(src_array.key());

        for (const auto& dst : src_array.value().items()){
            auto dst_key = std::stoi(dst.key());
            latency_map[{src_key,dst_key}] = dst.value();
        }
    }

    // set energy
    auto energy_array = latency_energy["energy"];
    for(const auto& src_array:energy_array.items()){
        auto src_key = std::stoi(src_array.key());

        for (const auto& dst : src_array.value().items()){
            auto dst_key = std::stoi(dst.key());
            energy_map[{src_key,dst_key}] = dst.value();
        }
    }
}

 void Network::readLatencyEnergyFile(const std::string &parent_path) {
    fs::path parent (parent_path);
    fs::path child (config.net_config_file_path);
    auto full_path = parent/child;

    std::ifstream f (full_path.string());

    auto json_file = nlohmann::json::parse(f);

    setLatencyEnergy(json_file);

 }
