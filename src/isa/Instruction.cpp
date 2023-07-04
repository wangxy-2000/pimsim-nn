//
// Created by xyfuture on 2023/6/30.
//
#include "Instruction.h"


std::map<Opcode,InstType> Instruction::op_to_type = {
        {Opcode::nop,InstType::nop},
        {Opcode::sldi,InstType::scalar},{Opcode::sld,InstType::scalar},{Opcode::sadd,InstType::scalar},
        {Opcode::ssub,InstType::scalar},{Opcode::smul,InstType::scalar},{Opcode::saddi,InstType::scalar},{Opcode::smuli,InstType::scalar},
        {Opcode::setbw,InstType::matrix}, // 特殊设计
        {Opcode::mvmul,InstType::matrix},
        {Opcode::vvadd,InstType::vector},{Opcode::vvsub,InstType::vector},{Opcode::vvmul,InstType::vector},
        {Opcode::vvdmul,InstType::vector},{Opcode::vvmax,InstType::vector},{Opcode::vvsll,InstType::vector},
        {Opcode::vvsra,InstType::vector},{Opcode::vavg,InstType::vector},{Opcode::vrelu,InstType::vector},
        {Opcode::vtanh,InstType::vector},{Opcode::vsigm,InstType::vector},{Opcode::vmv,InstType::vector},
        {Opcode::vrsu,InstType::vector},{Opcode::vrsl,InstType::vector},

        {Opcode::ld,InstType::transfer},{Opcode::st,InstType::transfer},{Opcode::lldi,InstType::transfer},
        {Opcode::lmv,InstType::transfer},{Opcode::send,InstType::transfer},{Opcode::recv,InstType::transfer},
        {Opcode::wait,InstType::transfer},{Opcode::sync,InstType::transfer},
};
