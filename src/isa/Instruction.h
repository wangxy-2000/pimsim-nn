//
// Created by xyfuture on 2023/3/1.
//


#pragma once
#ifndef INST_INSTRUCTION_H_
#define INST_INSTRUCTION_H_

#include <sstream>
#include <memory>
#include <map>
#include "nlohmann/json.hpp"
#include "isa/ISA.h"


#define RD_SELECT 1
#define RS1_SELECT 2
#define RS2_SELECT 4


struct OffsetField{
    int offset_value = 0;
    int offset_select = 0;
    // select function

    int getRdOffsetValue() const{
        if (offset_select & RD_SELECT)
            return offset_value;
        return 0;
    }

    int getRs1OffsetValue() const{
        if (offset_value & RS1_SELECT)
            return offset_value;
        return 0;
    }

    int getRs2OffsetValue() const{
        if (offset_value & RS2_SELECT)
            return offset_value;
        return 0;
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(OffsetField,offset_value,offset_select);
};

struct VectorField{
    int len = 0;
    OffsetField offset ;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VectorField,len,offset);
};

struct MatrixField{
    int mbiw = 0;
    int group = 0;

    int relu = 0;

    int ibiw = 0; // setbw inst use this
    int obiw = 0;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MatrixField,mbiw,group,relu,ibiw,obiw)
};

struct ScalarField{
    int imm = 0;
    int offset_value = 0; // byte

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ScalarField,imm,offset_value);
};

struct TransferField{
    int len = 0 ;
    int size = 0;
    int imm = 0;
    int core = 0; // send/recv/sync
    OffsetField offset ; // byte

    int wait_value = 0;
    int event_register = 0;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(TransferField,size,imm,len,core,offset,wait_value,event_register);

};

struct Instruction {
    // Common field
    Opcode op = Opcode::nop;
    int rs1_addr = 0;
    int rs2_addr = 0;
    int rd_addr = 0;

    // vector field
    std::shared_ptr<VectorField> vector = nullptr;
    // matrix field
    std::shared_ptr<MatrixField> matrix = nullptr;
    // scalar field
    std::shared_ptr<ScalarField> scalar = nullptr;
    // transfer field
    std::shared_ptr<TransferField> transfer = nullptr;

    InstType inst_type = InstType::nop;

    InstType getInstType() {
        if (inst_type == +InstType::nop) // default value, read again
            inst_type =  (*op_to_type.find(op)).second;
        return inst_type;
    }

    static std::map<Opcode,InstType> op_to_type;


    inline void readInst (const nlohmann::json & inst){
        auto j_op = inst.at("op").get<std::string>();
        op = Opcode::_from_string(j_op.c_str());

        auto type = getInstType();

        if (inst.count("rd") == 1){
            rd_addr = inst["rd"].get<int>();
        }
        if (inst.count("rs1") == 1){
            rs1_addr = inst["rs1"].get<int>();
        }
        if (inst.count("rs2") == 1){
            rs2_addr = inst["rs2"].get<int>();
        }


        switch (type) {
            case InstType::nop:
                break;
            case InstType::scalar:
                scalar = std::make_shared<ScalarField>(inst.get<ScalarField>());
                break;
            case InstType::matrix:
                matrix = std::make_shared<MatrixField>(inst.get<MatrixField>());
                break;
            case InstType::vector:
                vector = std::make_shared<VectorField>(inst.get<VectorField>());
                break;
            case InstType::transfer:
                transfer = std::make_shared<TransferField>(inst.get<TransferField>());
                break;
        }

    }

    Instruction(const nlohmann::json& json_inst){
        readInst(json_inst);
    }

    Instruction() = default;


    friend std::ostream & operator<<(std::ostream & os, const Instruction& inst) {
        os<<"inst type:"<<inst.inst_type._to_string()<<" opcode:"<<inst.op._to_string();

        if (inst.op == +Opcode::send or inst.op == +Opcode::recv){
            os <<" core_id:"<<inst.transfer->core;
        }

        os<<"\n";
        return os ;
    }


};



inline std::vector<Instruction> readSingleCoreInstFromJson(const nlohmann::json& json_inst_array){
    std::vector<Instruction> tmp;
    for (auto& cur_inst:json_inst_array){
        tmp.emplace_back(cur_inst);
    }
    return tmp;
}






#endif //INST_INSTRUCTION_H_
