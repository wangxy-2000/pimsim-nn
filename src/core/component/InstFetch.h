//
// Created by xyfuture on 2023/2/28.
//



#ifndef CORE_COMPONENT_INSTFETCH_H_
#define CORE_COMPONENT_INSTFETCH_H_
#include <vector>

#include <systemc>

#include "core/BaseCoreModule.h"
#include "isa/Instruction.h"
#include "utils/Register.hpp"
#include "core/payloads/StagePayloads.hpp"


using namespace sc_core;

// InstFetch : Instruction Fetch
// Update PC every cycle
// When in simulation mode 0 , instruction stream will be looped
class InstFetch: public BaseCoreModule  {
    SC_HAS_PROCESS(InstFetch);
public:
    InstFetch(const sc_module_name& name,const CoreConfig& config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk);

    void setInstBuffer(const std::vector<Instruction>& buffer);

    // use json as immediate inst format
    void readInstFromJson(const nlohmann::json & json_inst);

    // Update PC and Fetch Inst
    void process();

    std::string getStatus();

    bool isFinish();

    // store all instructions
    std::vector<Instruction> inst_buffer;
    int inst_buffer_size = 0;

private:
    RegEnable<int> pc_reg;

    sc_signal<int> pc_in;
    sc_signal<int> pc_out;
    sc_signal<bool> pc_reset;
public:

    sc_in<bool> if_enable;
    sc_out<bool> if_stall;

    sc_out<DecodeInfo> if_id_port;
};



#endif //CORE_COMPONENT_INSTFETCH_H_
