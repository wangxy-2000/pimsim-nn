//
// Created by xyfuture on 2023/3/1.
//

#pragma once

#include <string>
#include <systemc>
#include "isa/Instruction.h"
#include "isa/ISA.h"


#define MAKE_SIGNAL_TYPE_TRACE_STREAM(CLASS_NAME) \
friend std::ostream& operator<<(std::ostream& out,const CLASS_NAME & self ) { \
    out<<" "#CLASS_NAME" Type\n";             \
    return out;                           \
}                                         \
inline friend void sc_trace(sc_core::sc_trace_file* f, const CLASS_NAME& self,const std::string& name){}

template <typename T>
bool is_payload_valid(const T& payload){
    return !(payload.pc == -1 or payload.op == +Opcode::nop);
}

// discard
struct JumpInfo{
    MAKE_SIGNAL_TYPE_TRACE_STREAM(JumpInfo)

    bool is_jump = false;
    int offset=0;

};

struct DecodeInfo{
    MAKE_SIGNAL_TYPE_TRACE_STREAM(DecodeInfo)

    int pc ;
    Instruction inst;

    static DecodeInfo empty(){
        return {.pc=-1,.inst=Instruction()};
    }

    bool operator==(const DecodeInfo& ano) const{
        return ano.pc == pc;
    }

};


struct ScalarInfo {
    MAKE_SIGNAL_TYPE_TRACE_STREAM(ScalarInfo)

    int pc ;
    Opcode op ;

    int rd_addr ;

    int rs1_value;
    int rs2_value ;
    int imm;

    long long rs1_value_double ; //sld double word value

    static ScalarInfo empty(){
        return {.pc=-1,.op=Opcode::nop,.rd_addr=0,.rs1_value=0,.rs2_value=0,
                .imm=0,.rs1_value_double=0};
    }

    bool operator==(const ScalarInfo& ano) const{
        return std::tie(ano.pc, ano.op,ano.rd_addr, ano.rs1_value, ano.rs2_value, ano.imm, ano.rs1_value_double) ==
               std::tie(pc, op, rd_addr, rs1_value, rs2_value, imm, rs1_value_double);
        // c++20 support default, however we use c++17
    }
};


struct VectorInfo{
    MAKE_SIGNAL_TYPE_TRACE_STREAM(VectorInfo)

    int pc;
    Opcode op ;

    int rd_value;
    int rs1_value;
    int rs2_value ;

    int len;

    int input_bitwidth;
    int output_bitwidth ;

    static VectorInfo empty(){
        return {.pc=-1,.op=Opcode::nop,.rd_value=0,.rs1_value=0,.rs2_value=0,.len=0,.input_bitwidth=0,.output_bitwidth=0};
    }

    bool operator==(const VectorInfo& ano) const{
        return std::tie(ano.pc,ano.op,ano.rd_value,ano.rs1_value,ano.rs2_value,ano.len,ano.input_bitwidth,ano.output_bitwidth)
        == std::tie(pc,op,rd_value,rs1_value,rs2_value,len,input_bitwidth,output_bitwidth);
    }

};

struct MatrixInfo{
    MAKE_SIGNAL_TYPE_TRACE_STREAM(MatrixInfo)
    int pc ;
    Opcode op ;

    int rd_value;
    int rs1_value ;

    int mbiw ;
    int group ;
    int relu ;

    int input_bitwidth;
    int output_bitwidth ;

    static MatrixInfo empty(){
        return {.pc=-1,.op=Opcode::nop,.rd_value=0,.rs1_value=0,.mbiw=0,.group=0,.relu=0,.input_bitwidth=0,.output_bitwidth=0};
    }

    bool operator==(const MatrixInfo& ano) const{
        return std::tie(ano.pc,ano.op,ano.rd_value,ano.rs1_value,ano.mbiw,ano.group,ano.relu,ano.input_bitwidth,ano.output_bitwidth)
        == std::tie(pc,op,rd_value,rs1_value,mbiw,group,relu,input_bitwidth,output_bitwidth);
    }

};


struct TransferInfo{
    MAKE_SIGNAL_TYPE_TRACE_STREAM(TransferInfo)

    int pc ;
    Opcode op ;
    //
    int rd_value ;
    int rs1_value ;

    int imm;
    int core ;
    int size ;
    int event_register ;
    int wait_value;

    // double word mode
    long long rd_value_double;
    long long rs1_value_double ;

    static TransferInfo empty(){
        return {.pc=-1,.op=Opcode::nop,.rd_value=0,.rs1_value=0,.imm=0,.core=0,.size=0,.event_register=0,
                .wait_value=0,.rd_value_double=0,.rs1_value_double=0};
    }

    bool operator==(const TransferInfo& ano) const{
        return std::tie(ano.pc,ano.op,ano.rd_value,ano.rs1_value
        ,ano.imm,ano.core,ano.size,ano.event_register,ano.wait_value,ano.rd_value_double,ano.rs1_value_double)
        == std::tie(pc,op,rd_value,rs1_value,imm,core,size,event_register,wait_value,rd_value_double,rs1_value_double);
    }

};


struct RegFileReadAddr{
    MAKE_SIGNAL_TYPE_TRACE_STREAM(RegFileReadAddr)

    int rd_addr ;
    int rs1_addr ;
    int rs2_addr ;

    bool double_word ;

    static RegFileReadAddr empty(){
        return {.rd_addr=0,.rs1_addr=0,.rs2_addr=0,.double_word=false};
    }

    bool operator == (const RegFileReadAddr& ano) const{
        return std::tie(rd_addr,rs1_addr,rs2_addr,double_word)
        == std::tie(ano.rd_addr,ano.rs1_addr,ano.rs2_addr,ano.double_word);
    }
};

struct RegFileReadValue{
    MAKE_SIGNAL_TYPE_TRACE_STREAM(RegFileReadValue)

    int rd_value ;
    int rs1_value ;
    int rs2_value ;

    long long rd_value_double ;
    long long rs1_value_double ; // only rd and rs1 can be read as double

    static RegFileReadValue empty(){
        return {.rd_value=0,.rs1_value=0,.rs2_value=0,.rd_value_double=0,.rs1_value_double=0};
    }

    bool operator == (const RegFileReadValue& ano) const{
        return std::tie(rd_value,rs1_value,rs2_value,rd_value_double,rs1_value_double)
        == std::tie(ano.rd_value,ano.rs1_value,ano.rs2_value,ano.rd_value_double,ano.rs1_value_double);
    }
};

struct RegFileWrite{
    MAKE_SIGNAL_TYPE_TRACE_STREAM(RegFileWrite)

    int rd_addr ;
    int rd_value ;

    static RegFileWrite empty(){
        return {.rd_addr=0,.rd_value=0};
    }

    bool operator == (const RegFileWrite& ano) const {
        return rd_addr == ano.rd_addr && rd_value == ano.rd_value;
    }
};


struct BitwidthInfo{
    MAKE_SIGNAL_TYPE_TRACE_STREAM(BitwidthInfo)

    int input_bitwidth ; // bits
    int output_bitwidth ;

    int input_byte  ; // Bytes
    int output_byte ;

    static BitwidthInfo empty() {
        return {.input_bitwidth=8,.output_bitwidth=8,.input_byte=1,.output_byte=1};
    };

    bool operator == (const BitwidthInfo& ano) const{
        return input_bitwidth == ano.input_bitwidth && output_bitwidth==ano.output_bitwidth;
    }
};