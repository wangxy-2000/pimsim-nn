//
// Created by xyfuture on 2023/3/6.
//



#ifndef CORE_COMPONENT_REGFILE_H_
#define CORE_COMPONENT_REGFILE_H_

#include <vector>
#include <systemc>

#include "core/BaseCoreModule.h"
#include "core/payloads/StagePayloads.hpp"
#include "utils/Register.hpp"

using namespace sc_core;


// RegFile: Register Files
// Three Read Port - One Write Port
// Write Bypass Support
class RegFile: public BaseCoreModule{
    SC_HAS_PROCESS(RegFile);
public:
    RegFile(const sc_module_name& name,const CoreConfig& config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk);

    void readValue();
    void writeValue();
    void updateValue();


private:
    // for performance, not use register model for this component
    std::vector<int> reg_data;

private:
    RegNext<RegFileWrite> write_info_reg;

    sc_signal<RegFileWrite> reg_in;
    sc_signal<RegFileWrite> reg_out;

public:
    sc_in<RegFileReadAddr> reg_file_read_addr_port;
    sc_out<RegFileReadValue> reg_file_read_value_port;

    sc_in<RegFileWrite> reg_file_write_port;
};


#endif //CORE_COMPONENT_REGFILE_H_
