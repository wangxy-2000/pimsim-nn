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
    decode_reg.enable.bind(id_enable);

    bitwidth_reg.input.bind(bitwidth_reg_in);
    bitwidth_reg.output.bind(bitwidth_reg_out);
    bitwidth_reg.enable.bind(id_enable); // 同步更新


    SC_METHOD(checkStall);
    sensitive<<decode_reg_out<<matrix_busy_port<<vector_busy_port<<scalar_busy_port<<transfer_busy_port;

    SC_METHOD(readRegFile);
    sensitive<<decode_reg_out;

    SC_METHOD(decodeInst);
    sensitive<<decode_reg_out<<bitwidth_reg_out<<reg_file_read_value_port;

    energy_counter.setStaticPowerMW(config.inst_decode_static_power+config.inst_decode_offset_static_power);

}


void InstDecode::checkStall() {
    auto scalar_busy_info = scalar_busy_port.read();
    auto matrix_busy_info = matrix_busy_port.read();
    auto vector_busy_info = vector_busy_port.read();
    auto transfer_busy_info = transfer_busy_port.read();

    auto decode_info = decode_reg_out.read();

    auto stall_info = false; // default no stall the pipeline

    // if any exec unit is busy, then stall the whole pipeline
    // this will be improved in future
    if (matrix_busy_info or vector_busy_info or transfer_busy_info){
        stall_info = true;
    }

    // write to controller
    id_stall.write(stall_info);

}

void InstDecode::readRegFile() {
    // always read all register id (dst rs1 rs2)

    auto decode_info = decode_reg_out.read();
    auto inst = decode_info.inst;

    if (inst.op == +Opcode::nop) // not valid inst op
        return;

    RegFileReadAddr read_info = RegFileReadAddr{.rd_addr=inst.rd_addr,.rs1_addr=inst.rs1_addr,.rs2_addr=inst.rs2_addr};

    if (inst.op == +Opcode::sld || inst.op == +Opcode::ld || inst.op == +Opcode::st){
        read_info.double_word = true;
    }

    reg_file_read_addr_port.write(read_info);
}

void InstDecode::decodeInst() {
    auto decode_info = decode_reg_out.read();
    auto reg_read_value_info = reg_file_read_value_port.read();
    auto bitwidth_info = bitwidth_reg_out.read();

    ScalarInfo scalar_info = ScalarInfo::empty();
    MatrixInfo matrix_info = MatrixInfo::empty();
    VectorInfo vector_info = VectorInfo::empty();
    TransferInfo transfer_info = TransferInfo::empty();

    auto inst = decode_info.inst;
    auto pc = decode_info.pc;

//    std::cout<<getStatus();

    if (inst.getInstType() == +InstType::scalar){
        switch (inst.op) {
            case Opcode::sldi:
                scalar_info.rd_addr=inst.rd_addr;
                scalar_info.imm=inst.scalar->imm;
                break;
            case Opcode::sld:
                scalar_info.rd_addr=inst.rd_addr;
                scalar_info.rs1_value_double=reg_read_value_info.rs1_value_double + inst.scalar->offset_value;
                break;
            case Opcode::sadd:
                scalar_info.rd_addr=inst.rd_addr;
                scalar_info.rs1_value=reg_read_value_info.rs1_value;
                scalar_info.rs2_value=reg_read_value_info.rs2_value;
                break;
            case Opcode::ssub:
                scalar_info.rd_addr=inst.rd_addr;
                scalar_info.rs1_value=reg_read_value_info.rs1_value;
                scalar_info.rs2_value=reg_read_value_info.rs2_value;
                break;
            case Opcode::smul:
                scalar_info.rd_addr=inst.rd_addr;
                scalar_info.rs1_value=reg_read_value_info.rs1_value;
                scalar_info.rs2_value=reg_read_value_info.rs2_value;
                break;
            case Opcode::saddi:
                scalar_info.rd_addr=inst.rd_addr;
                scalar_info.rs1_value=reg_read_value_info.rs1_value;
                scalar_info.imm=inst.scalar->imm;
                break;
            case Opcode::smuli:
                scalar_info.rd_addr=inst.rd_addr;
                scalar_info.rs1_value=reg_read_value_info.rs1_value;
                scalar_info.imm=inst.scalar->imm;
                break;
            default:
                break;
        }
        // set pc and opcode for all scalar inst
        scalar_info.pc = pc;
        scalar_info.op = inst.op;
    }
    else if (inst.getInstType() == +InstType::matrix){

        if (inst.op == +Opcode::mvmul) {
            matrix_info = MatrixInfo{.pc=pc, .op=inst.op,
                    .rd_value=reg_read_value_info.rd_value, .rs1_value=reg_read_value_info.rs1_value,
                    .mbiw=inst.matrix->mbiw, .group=inst.matrix->group, .relu=inst.matrix->relu,
                    .input_bitwidth=bitwidth_info.input_bitwidth, .output_bitwidth=bitwidth_info.output_bitwidth};
        }
        // another matrix type inst is setbw
    }
    else if (inst.getInstType() == +InstType::vector){

        auto rd_offset = inst.vector->offset.getRdOffsetValue();
        auto rs1_offset = inst.vector->offset.getRs1OffsetValue();
        auto rs2_offset = inst.vector->offset.getRs2OffsetValue();

        auto rd_value = reg_read_value_info.rd_value;
        auto rs1_value = reg_read_value_info.rs1_value;
        auto rs2_value = reg_read_value_info.rs2_value;

        auto input_bitwidth = bitwidth_info.input_bitwidth;
        auto output_bitwidth = bitwidth_info.output_bitwidth;

        auto input_byte = bitwidth_info.input_byte;
        auto output_byte = bitwidth_info.output_byte;

        switch (inst.op) {
            case Opcode::vvadd:
                vector_info.rd_value = rd_value + input_byte * rd_offset;
                vector_info.rs1_value = rs1_value + input_byte * rs1_offset;
                vector_info.rs2_value = rs2_value + input_byte * rs2_value;
                break;
            case Opcode::vvsub:
                vector_info.rd_value = rd_value + input_byte * rd_offset;
                vector_info.rs1_value = rs1_value + input_byte * rs1_offset;
                vector_info.rs2_value = rs2_value + input_byte * rs2_value;
                break;
            case Opcode::vvmul:
                vector_info.rd_value = rd_value + output_byte * rd_offset;
                vector_info.rs1_value = rs1_value + input_byte * rs1_offset;
                vector_info.rs2_value = rs2_value + input_byte * rs2_offset;
                break;
            case Opcode::vvdmul:
                vector_info.rd_value = rd_value;
                vector_info.rs1_value = rs1_value + input_byte * rs1_offset;
                vector_info.rs2_value = rs2_value + input_byte * rd_offset;
                break;
            case Opcode::vvmax:
                vector_info.rd_value = rd_value + input_byte * rd_offset;
                vector_info.rs1_value = rs1_value + input_byte * rs1_offset;
                vector_info.rs2_value = rs2_value + input_byte * rs2_offset;
                break;
            case Opcode::vvsll:
                vector_info.rd_value=rd_value + output_byte * rd_offset;
                vector_info.rs1_value = rs1_value + input_byte * rs1_offset;
                vector_info.rs2_value = rs2_value + input_byte * rs2_offset;
                break;
            case Opcode::vvsra:
                vector_info.rd_value=rd_value + output_byte * rd_offset;
                vector_info.rs1_value=rs1_value + input_byte * rs1_offset;
                vector_info.rs2_value=rs2_value + input_byte * rs2_offset;
                break;
            case Opcode::vavg:
                vector_info.rd_value=rd_value;
                vector_info.rs1_value=rs1_value + input_byte*rs1_offset;
                vector_info.rs2_value=rs2_value;
                break;
            case Opcode::vrelu: // ISA的定义中似乎有些奇怪 全部受input bitwidth约束
                vector_info.rd_value=rd_value + input_byte*rd_offset;
                vector_info.rs1_value=rs1_value + input_byte*rs1_offset;
                break;
            case Opcode::vtanh: // 输入受input约束,输出受output约束
                vector_info.rd_value=rd_value + output_byte * rd_offset;
                vector_info.rs1_value=rs1_value + input_byte * rs1_offset;
                break;
            case Opcode::vsigm:
                vector_info.rd_value=rd_value + output_byte * rd_offset;
                vector_info.rs1_value=rs1_value + input_byte * rs1_offset;
                break;
            case Opcode::vmv:
                vector_info.rd_value=rd_value;
                vector_info.rs1_value=rs1_value;
                vector_info.rs2_value=rs2_value;
                break;
            case Opcode::vrsu:
                vector_info.rd_value=rd_value + output_byte * rd_offset;
                vector_info.rs1_value=rs1_value + input_byte * rs1_offset;
                vector_info.rs2_value=rs2_value;
                break;
            case Opcode::vrsl:
                vector_info.rd_value=rd_value + output_byte * rd_offset;
                vector_info.rs1_value=rs1_value + input_byte * rs1_offset;
                vector_info.rs2_value=rs2_value;
                break;
            default:
                break;
        }

        vector_info.pc = pc;
        vector_info.op = inst.op;
        vector_info.len = inst.vector->len; // 共有
        vector_info.input_bitwidth = input_bitwidth;
        vector_info.output_bitwidth = output_bitwidth;

    }
    else if (inst.getInstType() == +InstType::transfer){
        auto rd_value = reg_read_value_info.rd_value;
        auto rd_value_double = reg_read_value_info.rd_value_double;
        auto rs1_value = reg_read_value_info.rs1_value;
        auto rs1_value_double = reg_read_value_info.rs1_value_double;

        auto rd_offset = inst.transfer->offset.getRdOffsetValue();
        auto rs1_offset = inst.transfer->offset.getRs1OffsetValue();

        auto offset_byte = inst.transfer->offset.offset_value;

        switch (inst.op) {

            case Opcode::ld:
                // offset -> 1 byte / unit
                transfer_info.rd_value=rd_value + rd_offset;
                transfer_info.size=inst.transfer->size;
                transfer_info.rs1_value_double=rs1_value_double + rs1_offset;
                break;
            case Opcode::st:
                transfer_info.rs1_value=rs1_value + rs1_offset;
                transfer_info.size=inst.transfer->size;
                transfer_info.rd_value_double=rd_value_double + rd_offset;
                break;
            case Opcode::lldi:
                transfer_info.rd_value=rd_value + offset_byte;
                transfer_info.imm=inst.transfer->imm;
                transfer_info.size=inst.transfer->size;
                break;
            case Opcode::lmv:
                transfer_info.rd_value=rd_value + rd_offset;
                transfer_info.rs1_value=rs1_value + rs1_offset;
                transfer_info.size=inst.transfer->size;
                break;
            case Opcode::send: // 这里用rd取代规范中的rs1,因为域是rd的域
                transfer_info.rd_value=rd_value + rd_offset;
                transfer_info.core=inst.transfer->core;
                transfer_info.size=inst.transfer->size;
                break;
            case Opcode::recv:
                transfer_info.rd_value=rd_value + rd_offset;
                transfer_info.core=inst.transfer->core;
                transfer_info.size=inst.transfer->size;
                break;
            case Opcode::wait:
                transfer_info.event_register=inst.transfer->event_register;
                transfer_info.wait_value=inst.transfer->wait_value;
                break;
            case Opcode::sync:
                transfer_info.core=inst.transfer->core;
                transfer_info.event_register=inst.transfer->event_register;
                break;
            default:
                break;
        }
        transfer_info.pc = pc;
        transfer_info.op = inst.op;


    }
    else if (inst.getInstType() == +InstType::nop){
        // can not just return
        // need to pass default info to execute units
    }


    // exec setbw in InstDecode
    if (inst.op == +Opcode::setbw){
        auto input_bitwidth = inst.matrix->ibiw;
        auto output_bitwidth = inst.matrix->obiw;

        int input_byte = ceil(input_bitwidth*1.0/8.0);  // align to byte
        int output_byte = ceil(output_bitwidth*1.0/8.0);

        bitwidth_reg_in.write(BitwidthInfo{.input_bitwidth=input_bitwidth,.output_bitwidth=output_bitwidth,
                                           .input_byte=input_byte,.output_byte=output_byte});
    }


    // write to exec unit
    // every exec unit port will be written, if current inst is not fit for, just write nop info to its port
    id_scalar_port.write(scalar_info);
    id_matrix_port.write(matrix_info);
    id_vector_port.write(vector_info);
    id_transfer_port.write(transfer_info);


    // power info
    energy_counter.addDynamicEnergyPJ(0,sc_time_stamp(),core_config.period,
                                  core_config.inst_decode_dynamic_power+core_config.inst_decode_offset_dynamic_power);
}

std::string InstDecode::getStatus() {
    std::stringstream  s;

    auto decode_info = decode_reg_out.read();

    s<<"Decode>"<<" time:"<<sc_time_stamp()<<"\n"
    <<"pc:"<<decode_info.pc<<" op:"<<decode_info.inst.op._to_string()<<"\n\n";

    return s.str();

}

