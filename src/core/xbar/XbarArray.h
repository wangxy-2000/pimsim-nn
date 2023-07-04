//
// Created by xyfuture on 2023/5/7.
//



#ifndef CORE_XBAR_XBARARRAY_H_
#define CORE_XBAR_XBARARRAY_H_

#include <systemc>
#include "config/Config.hpp"

using namespace std;


class XbarArray {
public:
    explicit XbarArray(const MatrixUnitConfig& config_);

    // cycle for latency
    std::pair<int,double> getXbarArrayLatencyEnergy(int weight_precision,int input_precision,int output_precision);

    std::pair<int,int> getWeightShape(int weight_precision);
    int calcXbarArrayLatency(int weight_precision,int input_precision,int output_precision);
    double calcXbarArrayEnergy(int weight_precision,int input_precision,int output_precision);

    double getStaticPower();

private:
    double static_power = 0 ;// mW
    const MatrixUnitConfig& config;

};


#endif //CORE_XBAR_XBARARRAY_H_
