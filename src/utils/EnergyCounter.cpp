//
// Created by xyfuture on 2023/4/29.
//

#include "EnergyCounter.h"

EnergyCounter::EnergyCounter() {

}

EnergyCounter::EnergyCounter(const EnergyCounter & ano) {
    dynamic_energy = ano.dynamic_energy;
    static_power = ano.static_power;
    running_time = ano.running_time;
    is_set_running_time = ano.is_set_running_time;
    // not copy map
}

void EnergyCounter::addDynamicEnergyPJ(double energy) {
//    assert(energy>0);
    dynamic_energy += energy;
}

void EnergyCounter::addDynamicEnergyPJ(double latency, double power) {
//    assert(latency*power>0);
    dynamic_energy += latency*power;
}

void EnergyCounter::addDynamicEnergyPJ(int tag, const sc_core::sc_time& time, double energy) {
    if (!dynamic_time_tag.count(tag)){
        dynamic_time_tag[tag] = time;
        addDynamicEnergyPJ(energy);
    }
    else {
        if (dynamic_time_tag[tag] != time)
            addDynamicEnergyPJ(energy);
    }
}

void EnergyCounter::addDynamicEnergyPJ(int tag, const sc_core::sc_time& time, double latency, double power) {
    if (!dynamic_time_tag.count(tag)){
        dynamic_time_tag[tag] = time;
        addDynamicEnergyPJ(latency,power);
    }
    else {
        if (dynamic_time_tag[tag] != time)
            addDynamicEnergyPJ(latency,power);
    }
}



void EnergyCounter::setStaticPowerMW(double power) {
//    assert(power>0);
    static_power = power;
}

void EnergyCounter::setRunningTimeNS(double time) {
    assert(time > 0);
    running_time = time;
    is_set_running_time = true;
}

double EnergyCounter::getTotalEnergyPJ() const {
    return getStaticEnergyPJ()+getDynamicEnergyPJ();
}

double EnergyCounter::getDynamicEnergyPJ() const {
    return dynamic_energy;
}

double EnergyCounter::getStaticEnergyPJ() const {
    if (is_set_running_time)
        return static_power * running_time; // mW * ns = pJ
    return  -1;
}

double EnergyCounter::getAveragePowerMW() const {
    if (is_set_running_time)
        return getTotalEnergyPJ()/running_time;
    return -1;
}



double EnergyCounter::getRunningTimeNS() const {
    if (is_set_running_time)
        return running_time;
    return -1;
}

EnergyCounter &EnergyCounter::operator+=(const EnergyCounter &ano) {
    dynamic_energy += ano.dynamic_energy;
//    static_energy += ano.static_energy;
    static_power += ano.static_power;

    // no running time

    return *this;

}

void EnergyCounter::setRunningTimeNS(const sc_core::sc_time &time) {
    setRunningTimeNS(time.to_seconds()*1e9);
}

void EnergyCounter::initialize() {
    static_power = 0;
    dynamic_energy = 0;
    dynamic_time_tag = std::map<int,sc_core::sc_time> ();
    running_time = 0;
    is_set_running_time = false;
}



