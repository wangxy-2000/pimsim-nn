//
// Created by xyfuture on 2023/6/19.
//

//
// Created by xyfuture on 2023/6/7.
//
#include<fstream>
#include "chip/Chip.h"
#include "Simulator.h"
#include <zstr.hpp>

int main(){
    auto json_file_path = "D:\\code\\ScPIMsim\\test\\resnet18\\small.json";
    auto gz_file_path = "D:\\code\\ScPIMsim\\test\\resnet18\\small.gz";

    auto config_file_path = "D:\\code\\ScPIMsim\\test\\resnet18\\config.json";

    std::ifstream config_file(config_file_path);



//    std::ifstream json_file (json_file_path);
    zstr::ifstream json_file (json_file_path,std::ios::binary);
    zstr::ifstream gz_file (gz_file_path,std::ios::binary);

    std::cout<<"Load Inst and Config"<<std::endl;
    nlohmann::json json_inst = nlohmann::json::parse(json_file);
    nlohmann::json gz_inst = nlohmann::json::parse(gz_file);
    nlohmann::json json_config = nlohmann::json::parse(config_file);

    if (gz_inst == json_inst)
        std::cout<<"pass"<<std::endl;

    GlobalConfig config = json_config.get<GlobalConfig>();
    Chip chip_std (config.chip_config,config.sim_config);
//    Chip chip_zstr (config.chip_config,config.sim_config);

//    std::cout<<"Read Inst From Json"<<std::endl;
//    chip.readInstFromJson(json_inst);

    chip_std.readInstFromJson(gz_inst);
//    chip_zstr.readInstFromJson(json_cmp_inst);

//    for(int i=0;i<config.chip_config.core_cnt;i++){
//        if (chip_std.core_array[i]->inst_fetch.inst_buffer.size() != chip_zstr.core_array[i]->inst_fetch.inst_buffer.size())
//            std::cout<<"Size not Match"<<std::endl;
//        for (int j=0;j<chip_std.core_array[i]->inst_fetch.inst_buffer.size();j++){
//            assert(chip_std.core_array[i]->inst_fetch.inst_buffer[j] == chip_zstr.core_array[i]->inst_fetch.inst_buffer[j]);
//        }
//    }
//    std::cout<<"all pass";

    std::cout<<"Start Simulation"<<std::endl;
    sc_start(config.sim_config.sim_time,sc_core::SC_MS);

    std::cout<<chip_std.getSimulationReport()<<std::endl;

}