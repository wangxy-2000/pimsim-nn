//
// Created by xyfuture on 2023/3/30.
//



#ifndef CORE_CORE_H_
#define CORE_CORE_H_


#include <systemc>
#include "config/Config.hpp"
#include "memory/Memory.h"
#include "isa/Instruction.h"
#include "core/component/Components.h"
#include "utils/EnergyCounter.h"

using namespace sc_core;

class Chip;

// Core: contains core components(units), local memory and switch
class Core:sc_module {
    SC_HAS_PROCESS(Core);
public:
    Core(const sc_module_name& name,const CoreConfig& core_config_,const SimConfig& sim_config_,int core_id,Chip* chip,ClockDomain* clk);

    // two methods to initialize inst buffer, use the second one currently
    void setInstBuffer(const std::vector<Instruction>& inst_buffer);
    void readInstFromJson(const nlohmann::json& json_inst);
    void switchBind(Network* network);

    // when sim_mode = 1 use these three functions
    void setCompoEndPC(int pc);
    bool isFinish() const;
    void setFinish();

    sc_time getFinishTime();

    void addRunRounds();
    int getRunRounds() const;

    int getCoreID() const;


    void setEnergyCounter();
    EnergyCounter getEnergyCounter();

    std::string getSimulationReport();

public:

    Controller controller;
    InstFetch inst_fetch;
    InstDecode inst_decode;

    RegFile reg_file;
    ScalarUnit scalar_unit;

    MatrixUnit matrix_unit;
    VectorUnit vector_unit;
    TransferUnit transfer_unit;

    Switch core_switch;

    Memory local_memory;

private:
    const int core_id;
    const CoreConfig& core_config;
    const SimConfig& sim_config;

private:
    sc_signal<DecodeInfo> fetch_decode;
    sc_signal<ScalarInfo> decode_scalar;
    sc_signal<VectorInfo> decode_vector;
    sc_signal<MatrixInfo> decode_matrix;
    sc_signal<TransferInfo> decode_transfer;

    sc_signal<RegFileReadAddr> decode_register;
    sc_signal<RegFileReadValue> register_decode;
    sc_signal<RegFileWrite> scalar_register;

    sc_signal<bool> vector_busy_decode;
    sc_signal<bool> matrix_busy_decode;
    sc_signal<bool> transfer_busy_decode;
    sc_signal<bool> scalar_busy_decode;

    sc_signal<bool> decode_stall_controller;
    sc_signal<bool> fetch_stall_controller;

    sc_signal<bool> controller_enable_decode;
    sc_signal<bool> controller_enable_fetch;

private:
    bool finish_running = false;
    sc_time finish_time_stamp ;

    int run_rounds = 0;

private:
    EnergyCounter energy_counter;

private:
    Chip* chip_ptr;
    ClockDomain* clk_ptr;


};


#endif //CORE_CORE_H_
