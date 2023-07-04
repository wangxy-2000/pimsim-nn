//
// Created by xyfuture on 2023/3/1.
//

#pragma once

#ifndef CONFIG_BUSCONFIG_H_
#define CONFIG_BUSCONFIG_H_

#include <string>
#include <vector>
#include "nlohmann/json.hpp"

// check gt zero or gt_eq zero
template<typename T>
bool check_positive(const T& t) { // recursion endpoint
    if (t <= 0) return false;

    return true;
}

template<typename T, typename... Args>
bool check_positive(const T& t, const Args&... args) {
    bool ans = true;
    if(t<=0) ans = false;
    return  ans && check_positive(args...);
}

template<typename T>
bool check_not_negative(const T& t) {
    if (t < 0) return false;

    return true;
}

template<typename T, typename... Args>
bool check_not_negative(const T& t, const Args&... args) {
    bool ans = true;
    if(t<0) ans = false;
    return  ans && check_not_negative(args...);
}



class BusConfig {
public:

    std::string bus_topology = "mesh";
    int bus_width = 16; // Bytes


    std::pair<int,int> layout = {0,0}; // mesh use this

    // use ns and nJ as unit in latency and energy file
    // relative path
    std::string net_config_file_path = std::string ("");

    void checkValid(){
        auto ans = check_positive(bus_width,layout.first,layout.second);
        if (!ans) throw "BusConfig not valid";
    }
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BusConfig,
                                       bus_topology,bus_width,layout,net_config_file_path)

};


struct MemoryConfig{

    long long memory_size = 1024*1024 ; // Bytes
    int data_width = 16; // Bytes

    double period = 1; // ns

    double static_power = 1; // mW

    int write_latency_cycle = 10; // Cycles
    double write_dynamic_power = 1.0; // mW

    int read_latency_cycle = 8; // Cycles
    double read_dynamic_power = 1.0; // mW

    void checkValid(){
        auto ans = check_positive(memory_size,data_width,period,static_power,write_latency_cycle,write_dynamic_power,read_latency_cycle,read_dynamic_power);
        if (!ans) throw "MemoryConfig not valid";
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MemoryConfig,
                                                memory_size, data_width,
                                                period, static_power,
                                                write_latency_cycle, write_dynamic_power,
                                                read_latency_cycle, read_dynamic_power)
};


struct MatrixUnitConfig{
    int xbar_array_count = 1;

    double period = 1/1.28; // ns

    bool pipeline_mode = true;

    int dac_resolution = 1; // bit
    int dac_latency_cycle = 1; // ns
    double dac_static_power = 0; // mW
    double dac_dynamic_power = 0; // mW
    int dac_count = 128; // pics

    std::pair<int,int> xbar_size = {128,128};
    int cell_precision = 2; // bit
    double xbar_latency = 30; // ns whole xbar
    double xbar_read_power = 0; // mW

    int sample_hold_latency_cycle = 1; // ns
    double sample_hold_static_power = 0; // mW
    double sample_hold_dynamic_power = 0; // mW

    int adc_resolution = 1  ; // bit
    int adc_latency_cycle = 1; // cycles
    double adc_static_power = 0; // mW
    double adc_dynamic_power = 0; // mW
    int adc_count = 1 ;

    int shift_adder_latency_cycle = 1; // cycles
    double shift_adder_static_power = 0; // mW
    double shift_adder_dynamic_power = 0; // mW

    // reg
    int input_buffer_latency_cycle = 0; // read data size fit for dac count
    double input_buffer_static_power = 0;
    double input_buffer_dynamic_power = 0;

    int output_buffer_latency_cycle = 1; // cycles
    double output_buffer_static_power = 0;
    double output_buffer_dynamic_power = 0;

    void checkValid(){
        auto ans = check_not_negative(
                xbar_array_count,period,
                dac_resolution,dac_latency_cycle,dac_static_power,dac_dynamic_power,dac_count,
                xbar_size.first,xbar_size.second,cell_precision,xbar_latency,xbar_read_power,
                sample_hold_latency_cycle,sample_hold_static_power,sample_hold_dynamic_power,
                adc_resolution,adc_latency_cycle,adc_static_power,adc_dynamic_power,adc_count,
                shift_adder_latency_cycle,shift_adder_static_power,shift_adder_dynamic_power,
                input_buffer_static_power,input_buffer_dynamic_power,
                output_buffer_latency_cycle,output_buffer_static_power,output_buffer_dynamic_power
                );

        if (!ans) throw "MatrixConfig not valid";
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MatrixUnitConfig,xbar_array_count,period,pipeline_mode,
                    dac_resolution,dac_latency_cycle,dac_static_power,dac_dynamic_power,dac_count,
                    xbar_size,cell_precision,xbar_latency,xbar_read_power,
                    sample_hold_latency_cycle,sample_hold_static_power,sample_hold_dynamic_power,
                    adc_resolution,adc_latency_cycle,adc_static_power,adc_dynamic_power,adc_count,
                    shift_adder_latency_cycle,shift_adder_static_power,shift_adder_dynamic_power,
                    input_buffer_static_power,input_buffer_dynamic_power,
                    output_buffer_latency_cycle,output_buffer_static_power,output_buffer_dynamic_power);
};


struct CoreConfig{

//    double frequency = 1; //GHz
    double period = 1; // ns

    // back
//    int input_precision = 8 ; // bits
//    int weight_precision = 8; // bits

    double controller_static_power = 0; // mW
    double controller_dynamic_power = 0;

    double inst_fetch_static_power = 0; // mW
    double inst_fetch_dynamic_power = 0; // mW

    double inst_decode_static_power = 0; // mW
    double inst_decode_dynamic_power = 0; // mW
    double inst_decode_offset_static_power = 0; // mW
    double inst_decode_offset_dynamic_power = 0; // mW

    // 是否要把读写分开
    double reg_file_static_power = 0 ; //mW
    double reg_file_read_dynamic_power = 0; //mW
    double reg_file_write_dynamic_power = 0; //mW

    double scalar_static_power = 0; // mW
    double scalar_dynamic_power = 0; //mW

    // Matrix Unit
//    int matrix_latency = 1000; // Cycles
//    float matrix_energy_per_pe = 1.0; // nJ
//    int device_precision = 2; // bits
//    std::vector<int> xbar_size = {128,128};
//    std::vector<int> xbar_array = {2,2}; // 2x2 x 128x128
    MatrixUnitConfig matrix_config;

    // Vector Unit
    int vector_width = 32 ;
    int vector_latency_cycle = 4 ; // Cycles per op
    double vector_alu_static_power = 1.0; // mW
    double vector_alu_dynamic_power = 1.0; //mW
    double vector_reg_static_power = 1.0; //mW
    double vector_reg_dynamic_power = 1.0; //mW

    double transfer_static_power = 0;
    double transfer_dynamic_power = 0;

    MemoryConfig local_memory_config;

    // global_memory_id
    int global_memory_switch_id = -10; // for global memory access


    void checkValid(){
        auto ans = check_not_negative(
                period,
                controller_static_power, controller_dynamic_power,
                inst_fetch_static_power, inst_fetch_dynamic_power,
                inst_decode_static_power, inst_decode_dynamic_power, inst_decode_offset_static_power, inst_decode_offset_dynamic_power,
                reg_file_static_power, reg_file_read_dynamic_power, reg_file_write_dynamic_power,
                scalar_static_power, scalar_dynamic_power,
                vector_width, vector_latency_cycle, vector_alu_static_power, vector_alu_dynamic_power,
                vector_reg_static_power, vector_alu_dynamic_power,
                transfer_static_power, transfer_dynamic_power
                );
        local_memory_config.checkValid();
        matrix_config.checkValid();

        if (!ans) throw "CoreConfig not valid";
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CoreConfig,
                                                period,
//                        input_precision,weight_precision,
                        controller_static_power, controller_dynamic_power,
                                                inst_fetch_static_power, inst_fetch_dynamic_power,
                                                inst_decode_static_power, inst_decode_dynamic_power, inst_decode_offset_static_power, inst_decode_offset_dynamic_power,
                                                reg_file_static_power, reg_file_read_dynamic_power, reg_file_write_dynamic_power,
                                                scalar_static_power, scalar_dynamic_power,
                                                matrix_config,
                                                vector_width, vector_latency_cycle, vector_alu_static_power, vector_alu_dynamic_power,
                                                vector_reg_static_power, vector_alu_dynamic_power,
                                                transfer_static_power, transfer_dynamic_power,
                                                local_memory_config, global_memory_switch_id
                );
};


struct ChipConfig{
    CoreConfig core_config;
    MemoryConfig global_memory_config;
    BusConfig network_config;

    int core_cnt = 1;

    int global_memory_switch_id = -10;

    void checkValid(){
        auto ans = check_positive(core_cnt);

        core_config.checkValid();
        global_memory_config.checkValid();
        network_config.checkValid();

        if(!ans) throw "ChipConfig not valid";
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ChipConfig,
                                                core_config,global_memory_config,network_config,
                                                core_cnt,
                                                global_memory_switch_id);

};


struct SimConfig{
    int sim_mode=0; // 0 for run until specific time
                    // 1 for run just one round (all instructions)
    double sim_time = 1; //micro seconds
    int report_verbose_level=0;

    void checkValid(){
        auto ans = check_positive(sim_time) && check_not_negative(report_verbose_level) && (sim_mode == 1 || sim_mode == 0);
        if (!ans) throw "SimConfig not valid ";
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SimConfig,
                                                sim_mode, sim_time, report_verbose_level);
};


struct GlobalConfig{
    ChipConfig chip_config = ChipConfig();
    SimConfig sim_config = SimConfig();

    void checkValid(){
        chip_config.checkValid();
        sim_config.checkValid();
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(GlobalConfig,chip_config,sim_config);
};

#endif //CONFIG_BUSCONFIG_H_
