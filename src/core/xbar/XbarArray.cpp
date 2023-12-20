//
// Created by xyfuture on 2023/5/7.
//

#include "XbarArray.h"

XbarArray::XbarArray(const MatrixUnitConfig &config_)
:config(config_){

}

std::pair<int, double>
XbarArray::getXbarArrayLatencyEnergy(int weight_precision, int input_precision, int output_precision) {
    // compute one pipeline stage time
    // compute how many stages
    // compute total energy consumption and latency

    return std::make_pair(calcXbarArrayLatency(weight_precision,input_precision,output_precision),
                          calcXbarArrayEnergy(weight_precision,input_precision,output_precision));
}

double XbarArray::getStaticPower() {
     static_power = config.input_buffer_static_power+
            config.dac_static_power*config.dac_count+
            config.sample_hold_static_power+
            config.adc_static_power*config.adc_count+
            config.shift_adder_static_power * config.adc_count+
            config.output_buffer_static_power;
    return static_power;
}

int XbarArray::calcXbarArrayLatency(int weight_precision, int input_precision, int output_precision) {
//    auto weight_shape = getWeightShape(weight_precision);

    int total_latency_cycle = 0;

    int input_times = divideWithCeiling(input_precision,config.dac_resolution);
    // one input @ dac_precision
    int dac_times = divideWithCeiling(config.xbar_size.first,config.dac_count);
    int adc_times = divideWithCeiling(config.xbar_size.second,config.adc_count);


    int xbar_read_cycle = int(ceil(config.xbar_latency/config.period));

    // dac work once
    int single_front_stage_latency_cycle = config.input_buffer_latency_cycle +
                                            config.dac_latency_cycle +
                                            xbar_read_cycle +
                                            config.sample_hold_latency_cycle ;

    // adc read out all xbar values @ dac work once
    int back_stage_pipe_cycle = std::max(config.adc_latency_cycle, config.shift_adder_latency_cycle+config.output_buffer_latency_cycle);
    int single_back_stage_latency_cycle = config.adc_latency_cycle+config.shift_adder_latency_cycle+config.output_buffer_latency_cycle+
                                            (adc_times-1)*back_stage_pipe_cycle;


    if(config.pipeline_mode){
        int stage_pipe_cycle = std::max(single_front_stage_latency_cycle, single_back_stage_latency_cycle);

        int total_times = input_times * dac_times;
        total_latency_cycle = single_front_stage_latency_cycle + single_back_stage_latency_cycle +
                            (total_times - 1) * stage_pipe_cycle;
    }
    else {
        // no pipeline at all (no double sample&hold design)
        int total_times = input_times * dac_times;

        total_latency_cycle = (single_front_stage_latency_cycle + single_back_stage_latency_cycle) * total_times;
    }

    return total_latency_cycle;
}

std::pair<int, int> XbarArray::getWeightShape(int weight_precision) {
    auto xbar_shape = config.xbar_size;
    xbar_shape.second = int((config.cell_precision*xbar_shape.second)/weight_precision);
    return xbar_shape;
}

double XbarArray::calcXbarArrayEnergy(int weight_precision, int input_precision, int output_precision) {

    // one input power
    // total power

    double total_energy = 0 ;

    int input_times = divideWithCeiling(input_precision,config.dac_resolution);
    // one input @ dac_precision
    int dac_times = divideWithCeiling(config.xbar_size.first,config.dac_count);
    int adc_times = divideWithCeiling(config.xbar_size.second,config.adc_count);

    double front_stage_energy = config.period * config.input_buffer_dynamic_power * config.input_buffer_latency_cycle
                                + config.period * config.dac_dynamic_power * config.dac_latency_cycle * config.dac_count
                                + config.xbar_read_power * config.xbar_latency
                                + config.period * config.sample_hold_dynamic_power * config.sample_hold_latency_cycle * config.xbar_size.second;

    double back_stage_energy = adc_times * (
                config.period * config.adc_dynamic_power * config.adc_latency_cycle * config.adc_count
                + config.period * config.shift_adder_dynamic_power * config.shift_adder_latency_cycle
                + config.period * config.output_buffer_dynamic_power * config.output_buffer_latency_cycle
            );

    total_energy = (input_times * dac_times) * (front_stage_energy + back_stage_energy);

    return total_energy;
}

int XbarArray::divideWithCeiling(int numerator, int denominator) {
    if (denominator == 0) {
        std::cerr << "Error: Division by zero!" << std::endl;
        return 0;
    }

    int quotient = numerator / denominator;
    int remainder = numerator % denominator;

    if (remainder != 0 && ((numerator < 0) != (denominator < 0))) {
        quotient++;
    }
    return quotient;
}

