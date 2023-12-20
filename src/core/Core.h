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
    Core(const sc_module_name& name, const CoreConfig& core_config_, const SimConfig& sim_config_,
         int core_id_, const std::vector<int>& array_group_map_, Chip* chip, ClockDomain* clk);

    // two methods to initialize inst buffer, use the second one currently
    void setInstBuffer(const std::vector<Instruction>& inst_buffer);
    void readInstFromJson(const nlohmann::json& json_inst);
    void switchBind(Network* network);


    int getCoreID() const;
    const SimConfig& getSimConfig() const;
    const CoreConfig& getCoreConfig() const;
    const std::vector<int>& getArrayGroupMap() const;
    int getArrayGroupCount() const;


    void setEnergyCounter();
    EnergyCounter getEnergyCounter();

    std::string getSimulationReport();

    std::string getStatus();

public:

//    Controller controller;
    InstFetch inst_fetch;
    InstDecode inst_decode;
    Dispatcher dispatcher;

    ReorderBuffer reorder_buffer;

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

    int array_group_cnt;
    std::vector<int> array_group_map;

private:
    sc_signal<VP<DecodeInfo>> fetch_decode;

    sc_signal<VP<ScalarInfo>> decode_scalar;
    sc_signal<VP<ExecInfo>> decode_dispatch;

    sc_signal<VP<ExecInfo>> dispatch_out;

    sc_signal<RegFileReadAddr> decode_register;
    sc_signal<RegFileReadValue> register_decode;
    sc_signal<RegFileWrite> scalar_register;

    sc_signal<bool> decode_fetch_ready;
    sc_signal<bool> dispatch_decode_ready;

    sc_signal<bool> scalar_decode_ready;

    sc_signal<bool> vector_dispatch_ready;
    sc_signal<bool> matrix_dispatch_ready;
    sc_signal<bool> transfer_dispatch_ready;


//    sc_signal<VP<ExecInfo>> dispatch_rob_new;
    sc_signal<ExecInfo> dispatch_rob_pend;
    sc_signal<bool> rob_dispatch_is_conflict;
    sc_signal<bool> rob_dispatch_ready;

    sc_signal<ExecInfo> matrix_rob_commit;
    sc_signal<ExecInfo> vector_rob_commit;
    sc_signal<ExecInfo> transfer_rob_commit;

protected:
    void start_of_simulation () override;


public:
    sc_time getFinishTime();
    void setFinish();
    bool isFinish() const;
    int getMaxPC() const;

    void addRunRounds();
    int getRunRounds() const;

private:
    bool finish_running = false;
    sc_time finish_time_stamp ;
    int max_pc = 0;

    int run_rounds = 0;

private:
    EnergyCounter energy_counter;

private:
    Chip* chip_ptr;
    ClockDomain* clk_ptr;

};


#endif //CORE_CORE_H_
