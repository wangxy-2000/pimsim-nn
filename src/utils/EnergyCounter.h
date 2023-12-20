//
// Created by xyfuture on 2023/4/29.
//



#ifndef UTILS_PERFORMANCECOUNTER_H_
#define UTILS_PERFORMANCECOUNTER_H_

#include <systemc>
#include <map>


class EnergyCounter {
    // pJ ns mW -- default unit
public:
    EnergyCounter();

    EnergyCounter(const EnergyCounter &);

    void addDynamicEnergyPJ(double energy);
    void addDynamicEnergyPJ(double latency,double power);

    void addDynamicEnergyPJ(int tag,const sc_core::sc_time& time,double energy);
    void addDynamicEnergyPJ(int tag,const sc_core::sc_time& time,double latency,double power);

    void setStaticPowerMW(double power); // energy calc when setRunningTime

    void setRunningTimeNS(double time);
    void setRunningTimeNS(const sc_core::sc_time& time);
    double getRunningTimeNS() const;

    double getTotalEnergyPJ() const;
    double getDynamicEnergyPJ() const;
    double getStaticEnergyPJ() const;

    double getAveragePowerMW() const;

    void initialize();
public:
    EnergyCounter& operator += (const EnergyCounter& ano);
    friend std::ostream& operator << (std::ostream& out,const EnergyCounter& counter);



public:

    double static_power = 0; // mW

    double dynamic_energy = 0; // pJ

    std::map<int,sc_core::sc_time> dynamic_time_tag; // for dynamic_energy_record;

    double running_time = 0; // ns
    bool is_set_running_time = false;
};


#endif //UTILS_PERFORMANCECOUNTER_H_
