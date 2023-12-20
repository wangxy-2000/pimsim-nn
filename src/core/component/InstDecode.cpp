//
// Created by xyfuture on 2023/3/6.
//

#include <cmath>
#include "InstDecode.h"
#include "core/Core.h"
#include <sstream>

InstDecode::InstDecode(const sc_module_name &name, const CoreConfig &config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk)
: BaseCoreModule(name,config,sim_config,core_ptr,clk),
  decode_reg("decode_reg",clk),
  bitwidth_reg("bitwidth_reg",clk){

    decode_reg.input.bind(if_id_port);
    decode_reg.output.bind(decode_reg_out);
    decode_reg.in_ready.bind(id_ready_port);
    decode_reg.out_ready.bind(self_ready);

    bitwidth_reg.input.bind(bitwidth_reg_in);
    bitwidth_reg.output.bind(bitwidth_reg_out);
    bitwidth_reg.enable.bind(self_ready); // 同步更新


    SC_METHOD(me_checkStall);
    sensitive << dispatcher_ready_port << scalar_ready_port;

    SC_METHOD(me_readRegFile);
    sensitive<<decode_reg_out;

    SC_METHOD(me_decodeInst);
    sensitive<<decode_reg_out<<bitwidth_reg_out<<reg_file_read_value_port;

    energy_counter.setStaticPowerMW(config.inst_decode_static_power+config.inst_decode_offset_static_power);

}


void InstDecode::me_checkStall() {
    auto scalar_busy_info = scalar_ready_port.read(); // not used
    auto dispatcher_ready_info = dispatcher_ready_port.read();

    auto cur_info = decode_reg_out.read();

    auto ready_info = dispatcher_ready_info; // default no stall the pipeline

    self_ready.write(ready_info);
}

void InstDecode::me_readRegFile() {
    // always read all register id (dst rs1 rs2)

    const auto& cur_info = decode_reg_out.read();
    if (not cur_info.valid) // not valid no need to read register
        return;

    const auto& decode_info = cur_info.payload;
    const auto& inst = decode_info.inst;

    if (inst.op == +Opcode::nop) // not valid inst op
        return;

    RegFileReadAddr read_info = RegFileReadAddr{.rd_addr=inst.rd_addr,.rs1_addr=inst.rs1_addr,.rs2_addr=inst.rs2_addr};

    if (inst.op == +Opcode::sld || inst.op == +Opcode::ld || inst.op == +Opcode::st){
        read_info.double_word = true;
    }

    reg_file_read_addr_port.write(read_info);
}

void InstDecode::me_decodeInst() {
    const auto& cur_info = decode_reg_out.read();
    const auto& reg_read_value_info = reg_file_read_value_port.read();
    const auto& bitwidth_info = bitwidth_reg_out.read();

    if (not cur_info.valid){ // pass invalid
        id_dispatch_port.write({ExecInfo(), false});
        id_scalar_port.write({ScalarInfo(),false});
        return;
    }

    auto decode_info = cur_info.payload;
    auto inst = decode_info.inst;
    auto pc = decode_info.pc;


    if(inst.op == +Opcode::setbw){
        // execute this inst in decode stage
        auto input_bitwidth = inst.matrix->ibiw;
        auto output_bitwidth = inst.matrix->obiw;

        int input_byte = ceil(input_bitwidth*1.0/8.0);  // align to byte
        int output_byte = ceil(output_bitwidth*1.0/8.0);

        bitwidth_reg_in.write(BitwidthInfo{.input_bitwidth=input_bitwidth,.output_bitwidth=output_bitwidth,
                .input_byte=input_byte,.output_byte=output_byte});

        // a bubble
        id_dispatch_port.write({ExecInfo(), false});
        id_scalar_port.write({ScalarInfo(),false});
    }
    else {
        // TODO check copy function
        ExecInfo exec_info (pc,
                            make_shared<Instruction>(inst),
                            make_shared<RegFileReadValue>(reg_read_value_info),
                            make_shared<BitwidthInfo>(bitwidth_info),
                            core_ptr);

        if (exec_info.exec_unit == +ExecType::Scalar){
            id_scalar_port.write({exec_info.getScalarInfo(),true});
            id_dispatch_port.write({ExecInfo(), false});
        } else {
            id_dispatch_port.write({exec_info, true});
            id_scalar_port.write({ScalarInfo(), false});
        }
    }

    // power info
    energy_counter.addDynamicEnergyPJ(0,sc_time_stamp(),core_config.period,
                                  core_config.inst_decode_dynamic_power+core_config.inst_decode_offset_dynamic_power);
}

std::string InstDecode::getStatus() {
    std::stringstream  s;

    s <<"Core:"<<core_ptr->getCoreID()<<" Decode>"<<" time: "<<sc_time_stamp()<<"\n";

    const auto& cur_info = decode_reg_out.read();
    if (cur_info.valid){
        auto decode_info = cur_info.payload;
         s <<"pc:"<<decode_info.pc<<" "<<decode_info.inst<<"\n\n";
    } else {
        s <<"invalid inst"<<"\n\n";
    }

    return s.str();

}

