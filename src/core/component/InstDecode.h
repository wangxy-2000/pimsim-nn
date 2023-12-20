//
// Created by xyfuture on 2023/3/6.
//



#ifndef CORE_COMPONENT_INSTDECODE_H_
#define CORE_COMPONENT_INSTDECODE_H_

#include <systemc>

#include "core/BaseCoreModule.h"
#include "isa/Instruction.h"
#include "utils/Register.hpp"
#include "core/payloads/StagePayloads.hpp"
#include "core/payloads/ExecInfo.h"

using namespace sc_core;



// InstDecode: Instruction Decode
// Decoding inst from InstFetch to ExecUnit
// Input: Inst - Output: UnitInfo
// Also process issue about bitwidth
class InstDecode: public BaseCoreModule{
    SC_HAS_PROCESS(InstDecode);
public:
    InstDecode(const sc_module_name& name,const CoreConfig& config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk);

    void me_readRegFile();
    void me_decodeInst();
    void me_checkStall(); // check stall info, whether this inst can be dispatched

    std::string getStatus();


private:
    RegPipe<VP<DecodeInfo>> decode_reg;
    sc_signal<VP<DecodeInfo>> decode_reg_out;

    sc_signal<bool>  self_ready;

private:
    // Bitwidth status
    RegEnable<BitwidthInfo> bitwidth_reg;
    sc_signal<BitwidthInfo> bitwidth_reg_in;
    sc_signal<BitwidthInfo> bitwidth_reg_out;

public:
//    sc_in<bool> id_enable;
    sc_out<bool> id_ready_port;
    sc_in<bool> dispatcher_ready_port;

    sc_in<VP<DecodeInfo>> if_id_port;

    // InstDecode to ExecUnit ports
    sc_out<VP<ScalarInfo>> id_scalar_port;
    sc_out<VP<ExecInfo>> id_dispatch_port;

    sc_out<RegFileReadAddr> reg_file_read_addr_port;
    sc_in<RegFileReadValue> reg_file_read_value_port;

    // ExecUnit busy port
    sc_in<bool> scalar_ready_port;


};


#endif //CORE_COMPONENT_INSTDECODE_H_
