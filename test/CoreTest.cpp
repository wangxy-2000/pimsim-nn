//
// Created by xyfuture on 2023/4/18.
//

#include <fstream>
#include "isa/Instruction.h"
#include "chip/Chip.h"
#include "systemc"

int sc_main(int, char*[]){

    std::ifstream f ("D:\\code\\ScPIMsim\\test\\inst\\vector.json");

    nlohmann::json json_inst = nlohmann::json::parse(f);
    std::vector<Instruction> inst_buffer = readSingleCoreInstFromJson(json_inst);

//    ChipConfig chip_config;
    GlobalConfig config;

    Chip chip (config.chip_config,config.sim_config);

    chip.core_array[0]->setInstBuffer(inst_buffer);


    std::cout<<"start simulation"<<std::endl;

    sc_start(100, sc_core::SC_SEC); // run simulation for 2 second

    std::cout<<"finish simulation"<<std::endl;
    return 0;
}
