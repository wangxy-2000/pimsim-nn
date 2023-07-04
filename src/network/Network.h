//
// Created by xyfuture on 2023/3/18.
//



#ifndef NETWORK_NETWORK_H_
#define NETWORK_NETWORK_H_

#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include "config/Config.hpp"
#include "utils/EnergyCounter.h"
#include <unordered_map>

class Switch;


struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        // Mainly for demonstration purposes, i.e. works but is overly simple
        // In the real world, use sth. like boost.hash_combine
        return (~h1) ^ h2;
    }
};


// Network : Network on chip (NoC)
class Network {
public:
    Network(const std::string& name,const BusConfig& config);

    // return latency and accumulate energy consumption
    int transfer(int src, int dst, int data_size);

    Switch* getSwitch(int switch_id);

    // add new switch to switch_map
    void registerSwitch(int switch_id,Switch* _switch);

    void readLatencyEnergyFile(const std::string& parent_path);
    void setLatencyEnergy(const nlohmann::json & latency_energy);

    EnergyCounter getEnergyCounter();
private:
    const BusConfig& config;
    std::string name;

private:
    std::map<int,Switch*> switch_map;

private:
//    std::map<std::pair<int,int>,double> latency_map; // ns
//    std::map<std::pair<int,int>,double> energy_map; // nJ

    std::unordered_map<std::pair<int,int>,double,pair_hash> latency_map; // ns
    std::unordered_map<std::pair<int,int>,double,pair_hash> energy_map; // nJ

private:
    EnergyCounter energy_counter;
};


#endif //NETWORK_NETWORK_H_
