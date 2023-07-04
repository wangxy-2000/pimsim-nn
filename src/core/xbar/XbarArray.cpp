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

    if (config.pipeline_mode){
        // pipeline mode
        int dac_times = config.xbar_size.first / config.dac_count;
        int xbar_read_cycle =  int(ceil(config.xbar_latency/config.period));
        int single_front_stage_latency_cycle = config.input_buffer_latency_cycle + config.dac_latency_cycle +
               + xbar_read_cycle + config.sample_hold_latency_cycle;

        int front_stage_latency_cycle = dac_times * single_front_stage_latency_cycle;
        int back_stage_latency_cycle = (config.xbar_size.second/config.adc_count) * config.adc_latency_cycle + config.shift_adder_latency_cycle + config.output_buffer_latency_cycle;

        auto pipeline_latency_cycle = front_stage_latency_cycle>back_stage_latency_cycle ? front_stage_latency_cycle:back_stage_latency_cycle;

        int input_times = ceil(input_precision / config.dac_resolution);

        total_latency_cycle = (input_times+1) * pipeline_latency_cycle + config.shift_adder_latency_cycle + config.output_buffer_latency_cycle;
    //    double total_latency = total_latency_cycle * config.period;
    }
    else{
        // plain mode
        int dac_times = config.xbar_size.first / config.dac_count;
        int adc_times = config.xbar_size.second / config.adc_count;

        int xbar_read_cycle = int(ceil(config.xbar_latency/config.period));

        int one_bit_cycle =config.input_buffer_latency_cycle +  dac_times * (config.dac_latency_cycle + xbar_read_cycle + config.sample_hold_latency_cycle) +
                adc_times * config.adc_latency_cycle + config.shift_adder_latency_cycle + config.output_buffer_latency_cycle;

        int input_times = ceil(input_precision / config.dac_resolution);

        total_latency_cycle = one_bit_cycle*input_times;
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

    int dac_times = config.xbar_size.first / config.dac_count;
    double dac_energy = config.dac_dynamic_power * config.dac_latency_cycle * config.period
                        * config.dac_count * dac_times;

    double xbar_read_energy = config.xbar_read_power * config.xbar_latency;

    double sample_hold_energy = config.sample_hold_dynamic_power * config.sample_hold_latency_cycle * config.period;

    int adc_times = config.xbar_size.second / config.adc_count;
    double adc_energy = config.adc_dynamic_power * config.adc_latency_cycle * config.period
                        * config.adc_count * adc_times;

    double shift_adder_energy = config.shift_adder_dynamic_power * config.shift_adder_latency_cycle * config.period * adc_times;

    double buffer_energy = (config.input_buffer_dynamic_power + config.output_buffer_dynamic_power*config.output_buffer_latency_cycle)* config.period;

    double one_bit_energy = dac_energy + xbar_read_energy + sample_hold_energy + adc_energy + shift_adder_energy + buffer_energy;

    int input_times = ceil(input_precision / config.dac_resolution);

    double total_energy = input_times * one_bit_energy;

    return total_energy;
}


