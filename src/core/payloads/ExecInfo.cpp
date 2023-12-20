//
// Created by xyfuture on 2023/8/3.
//

#include "ExecInfo.h"
#include "core/Core.h"

ExecInfo::ExecInfo(int pc, const std::shared_ptr<Instruction> &inst, const std::shared_ptr<RegFileReadValue> &reg_value,
                   const std::shared_ptr<BitwidthInfo> &bitwidth,Core* core_ptr= nullptr):
pc(pc),inst_ptr(inst),reg_value_ptr(reg_value),bitwidth_ptr(bitwidth),core_ptr(core_ptr){
    is_valid = true;
    parse();
}

void ExecInfo::parse() {
    if (inst_ptr == nullptr || reg_value_ptr == nullptr || bitwidth_ptr == nullptr)
        throw "Invalid ExecInfo";

    if ( pc == -1 || inst_ptr->getInstType() == +InstType::nop) // nop
        return;

    // set exec unit
    auto inst_type = inst_ptr->getInstType();
    if (inst_type == +InstType::scalar)
        exec_unit = ExecType::Scalar;
    else if (inst_type == +InstType::matrix)
        exec_unit = ExecType::Matrix;
    else if (inst_type == +InstType::vector)
        exec_unit = ExecType::Vector;
    else if (inst_type == +InstType::transfer)
        exec_unit = ExecType::Transfer;

    // set read/write memory addr
    setMemoryReadWriteAddr();

}

std::ostream &operator<<(std::ostream &out, const ExecInfo &info) {
    if (info.is_valid){
        out<<"ExecInfo- pc:"<<info.pc<<" "<<*info.inst_ptr;
        if (!info.memory_read_addr.empty()){
            out<<"  read addr:";
            for (auto p : info.memory_read_addr){
                out<<p.first<<"-"<<p.second<<"  ";
            }
            out<<"\n";
        }
        if (!info.memory_write_addr.empty()){
            out<< "  write addr:";
            for (auto p : info.memory_write_addr){
                out<<p.first<<"-"<<p.second<<"  ";
            }
            out<<"\n";
        }
    }
    else
        out <<"invalid exec_info\n";
    return out;
}


int ExecInfo::getFinalRS1() const{
    auto input_byte = bitwidth_ptr->input_byte;
    auto reg_rs1 = reg_value_ptr->rs1_value;
    auto inst_type = inst_ptr->getInstType();

    if (inst_type == +InstType::scalar){
        return reg_rs1;
    } else if (inst_type == +InstType::matrix){
        return reg_rs1;
    } else if (inst_type == +InstType::vector){
        auto rs1_offset = inst_ptr->vector->offset.getRs1OffsetValue();

        if (inst_ptr->op == +Opcode::vmv)
            return reg_rs1;

        return reg_rs1 + input_byte * rs1_offset;
    } else if (inst_type == +InstType::transfer){
        auto rs1_offset = inst_ptr->transfer->offset.getRs1OffsetValue();
        return reg_rs1 + rs1_offset;
    }

    // default
    return reg_rs1;
}

int ExecInfo::getFinalRS2() const{
    auto input_byte = bitwidth_ptr->input_byte;
    auto reg_rs2 = reg_value_ptr->rs2_value;
    auto inst_type = inst_ptr->getInstType();

    if (inst_type == +InstType::scalar){
        return reg_rs2;
    } else if (inst_type == +InstType::matrix){
        return reg_rs2; // actually not used
    } else if (inst_type == +InstType::vector){
        auto op = inst_ptr->op;
        auto rs2_offset = inst_ptr->vector->offset.getRs2OffsetValue();

        bool is_rs2_no_offset = op==+Opcode::vavg || op==+Opcode::vmv ||
                op==+Opcode::vrsu || op==+Opcode::vrsl;
        if (is_rs2_no_offset)
            return reg_rs2;

        return reg_rs2 + input_byte * rs2_offset;
    } else if (inst_type == +InstType::transfer){
        // not used in transfer
    }

    return reg_rs2;
}

int ExecInfo::getFinalRD() const{
    auto output_byte = bitwidth_ptr->output_byte;
    auto reg_rd = reg_value_ptr->rd_value;
    auto inst_type = inst_ptr->getInstType();

    if (inst_type == +InstType::scalar){
        // not used in scalar
    }
    else if (inst_type == +InstType::matrix){
        return reg_rd;
    }
    else if (inst_type == +InstType::vector){
        auto op = inst_ptr->op;
        auto rd_offset = inst_ptr->vector->offset.getRdOffsetValue();

        bool is_rd_no_offset = op==+Opcode::vvdmul || op==+Opcode::vavg || op==+Opcode::vmv;
        if (is_rd_no_offset){
            return reg_rd;
        }

        bool is_rd_use_rs1_bitwidth = op==+Opcode::vvadd || op==+Opcode::vvsub || op==+Opcode::vvmax || op==+Opcode::vrelu;
        if (is_rd_use_rs1_bitwidth){
            return reg_rd + bitwidth_ptr->input_byte * rd_offset;
        }
        // most case
        return reg_rd + output_byte*rd_offset;
    } else if (inst_type == +InstType::transfer){
        auto rd_offset = inst_ptr->transfer->offset.getRdOffsetValue();
        return reg_rd + rd_offset;
    }

    return reg_rd;
}

ll ExecInfo::getFinalRS1Double() const{
    auto reg_rs1_double = reg_value_ptr->rs1_value_double;
    auto op = inst_ptr->op;
    if (op == +Opcode::sld){
        return reg_rs1_double + inst_ptr->scalar->offset_value;
    } else if (op == +Opcode::ld){
        return reg_rs1_double + inst_ptr->transfer->offset.getRs1OffsetValue();
    }
    return 0;
}

ll ExecInfo::getFinalRDDouble() const{
    auto reg_rd_double = reg_value_ptr->rd_value_double;
    auto op = inst_ptr->op;
    if(op == +Opcode::st){
        return reg_rd_double + inst_ptr->transfer->offset.getRdOffsetValue();
    }

    return 0;
}

void ExecInfo::setMemoryReadWriteAddr() {
    auto inst_type = inst_ptr->getInstType();
    auto op = inst_ptr->op;

    auto rs1_value = getFinalRS1();
    auto rs2_value = getFinalRS2();
    auto rd_value = getFinalRD();

    if (inst_type == +InstType::scalar){
        // not read or write
    } else if (inst_type == +InstType::matrix) {
        // need to know xbar size
        auto read_addr = getFinalRS1();
        auto write_addr = getFinalRD();

        auto& matrix_config = core_ptr->getCoreConfig().matrix_config;

        auto read_len =matrix_config.xbar_size.first;
        auto array_group_id = inst_ptr->matrix->group;
        auto weight_precision = inst_ptr->matrix->mbiw;
        auto per_xbar_write_len = int(matrix_config.xbar_size.second*matrix_config.cell_precision*1.0 / weight_precision );
        auto write_len = per_xbar_write_len * core_ptr->getArrayGroupMap()[array_group_id]; //per xbar * xbar count

        int input_byte =  bitwidth_ptr->input_byte;
        int output_byte = bitwidth_ptr->output_byte;

        memory_read_addr.emplace_back(read_addr,read_addr + read_len*input_byte);
        memory_write_addr.emplace_back(write_addr,write_addr + write_len*output_byte);

    } else if (inst_type == +InstType::vector){
        auto len = inst_ptr->vector->len;
        auto input_byte = bitwidth_ptr->input_byte;
        auto output_byte = bitwidth_ptr->output_byte;

        if ( op==+Opcode::vrelu || op==+Opcode::vtanh || op==+Opcode::vsigm){
            memory_read_addr.emplace_back(rs1_value,rs1_value+len*input_byte);
            memory_write_addr.emplace_back(rd_value,rd_value+len*output_byte);
        } else if (op == +Opcode::vvdmul){
            memory_read_addr.emplace_back(rs1_value,rs1_value+len*input_byte);
            memory_read_addr.emplace_back(rs2_value,rs2_value+len*input_byte);

            memory_write_addr.emplace_back(rd_value,rd_value+len*output_byte);
        } else if (op == +Opcode::vavg){
            //TODO2
            memory_read_addr.emplace_back(rs1_value,rs1_value + len*input_byte);
            memory_write_addr.emplace_back(rd_value,rd_value+output_byte);
        } else if (op == +Opcode::vmv){

        } else if (op == +Opcode::vrsu || op == +Opcode::vrsl){
            memory_read_addr.emplace_back(rs1_value,rs1_value + len*input_byte);
            memory_read_addr.emplace_back(rs2_value,rs2_value + len*input_byte);

            memory_write_addr.emplace_back(rd_value, rd_value + len*output_byte);

        } else if (op == +Opcode::vvadd || op == +Opcode::vvsub || op == +Opcode::vvmax){

            memory_read_addr.emplace_back(rs1_value,rs1_value + len*input_byte);
            memory_read_addr.emplace_back(rs2_value,rs2_value + len*input_byte);

            memory_write_addr.emplace_back(rd_value,rd_value + len*input_byte);

        } else {
            memory_read_addr.emplace_back(rs1_value,rs1_value+len*input_byte);
            memory_read_addr.emplace_back(rs2_value,rs2_value+len*input_byte);

            memory_write_addr.emplace_back(rd_value,rd_value+len*output_byte);
        }


    } else if (inst_type == +InstType::transfer){
        auto imm_size = inst_ptr->transfer->size;
        if (op == +Opcode::ld){
            memory_write_addr.emplace_back(rd_value,rd_value+imm_size);
        } else if (op == +Opcode::st){
            memory_read_addr.emplace_back(rs1_value,rs1_value+imm_size);
        } else if (op == +Opcode::lldi){
            memory_write_addr.emplace_back(rd_value,rd_value+1);
        } else if (op == +Opcode::lmv){
            memory_read_addr.emplace_back(rs1_value,rs1_value+imm_size);
            memory_write_addr.emplace_back(rd_value,rd_value+imm_size);
        } else if (op == +Opcode::send){
            memory_read_addr.emplace_back(rs1_value,rs1_value+imm_size);
        } else if (op == +Opcode::recv){
            memory_write_addr.emplace_back(rd_value,rd_value+imm_size);
        } else {
            //  not read/write local memory
        }
    }
}

MatrixInfo ExecInfo::getMatrixInfo() const{
    MatrixInfo matrix_info {
        .pc = pc,.op=inst_ptr->op,
        .rd_value=getFinalRD(),.rs1_value=getFinalRS1(),
        .mbiw=inst_ptr->matrix->mbiw,.group=inst_ptr->matrix->group,.relu=inst_ptr->matrix->relu,
        .input_bitwidth=bitwidth_ptr->input_bitwidth,.output_bitwidth=bitwidth_ptr->output_bitwidth
    };
    return matrix_info;
}

VectorInfo ExecInfo::getVectorInfo() const{
    VectorInfo vector_info = VectorInfo::empty();
    auto op = inst_ptr->op;

    // common part
    vector_info.pc = pc;
    vector_info.op = op;
    vector_info.len = inst_ptr->vector->len;
    vector_info.input_bitwidth = bitwidth_ptr->input_bitwidth;
    vector_info.output_bitwidth = bitwidth_ptr->output_bitwidth;

    // set rs1 rs2 and rd
    vector_info.rs1_value = getFinalRS1();
    vector_info.rs2_value = getFinalRS2();
    vector_info.rd_value = getFinalRD();

    return vector_info;
}

TransferInfo ExecInfo::getTransferInfo() const{
    TransferInfo transfer_info = TransferInfo::empty();
    auto op = inst_ptr->op;

    // common part
    transfer_info.pc = pc;
    transfer_info.op = op;

    auto rd_value = getFinalRD();
    auto rs1_value = getFinalRS1();


    transfer_info.rd_value = rd_value;
    transfer_info.rs1_value = rs1_value;

    transfer_info.size = inst_ptr->transfer->size;
    transfer_info.imm = inst_ptr->transfer->imm;
    transfer_info.len = inst_ptr->transfer->len;

    transfer_info.core = inst_ptr->transfer->core;
    transfer_info.event_register = inst_ptr->transfer->event_register;
    transfer_info.wait_value = inst_ptr->transfer->wait_value;

    // less branch
    if (op == +Opcode::ld)
        transfer_info.rs1_value_double = getFinalRS1Double();
    else if (op == +Opcode::st)
        transfer_info.rd_value_double = getFinalRDDouble();

    return transfer_info;
}

ScalarInfo ExecInfo::getScalarInfo() const{
    auto rs1_value = getFinalRS1();
    auto rs2_value = getFinalRS2();
    auto rs1_value_double = getFinalRS1Double();
    ScalarInfo scalar_info = {
            .pc=pc,.op=inst_ptr->op,
            .rd_addr=inst_ptr->rd_addr,.rs1_value = rs1_value,.rs2_value=rs2_value,
            .imm=inst_ptr->scalar->imm,.rs1_value_double=rs1_value_double
    };

    return scalar_info;
}

ExecInfo::ExecInfo()
:pc(-1),exec_unit(ExecType::Nop),core_ptr(nullptr){
}

void sc_trace(sc_core::sc_trace_file *f, const ExecInfo &info, const std::string &name) {
}

bool ExecInfo::isConflict(const ExecInfo &ano) const{
    for (auto& cur_write_addr : memory_write_addr){
        for(auto& ano_read_addr:ano.memory_read_addr){
            // WAR
            if (isOverlap(cur_write_addr,ano_read_addr)) return true;
        }

        for (auto& ano_write_addr:ano.memory_read_addr){
            // WAW
            if (isOverlap(cur_write_addr,ano_write_addr)) return true;
        }
    }

    for (auto& cur_read_addr:memory_read_addr){
        for(auto& ano_write_addr:ano.memory_write_addr){
            // RAW
            if (isOverlap(cur_read_addr,ano_write_addr)) return true;
        }
    }

    return false;
}

bool ExecInfo::isValid() const {
    return is_valid;
}


// TODO change compare method

bool ExecInfo::operator==(const ExecInfo &ano) const {
    //TODO change compare element
    return pc == ano.pc and inst_ptr == ano.inst_ptr and reg_value_ptr==ano.reg_value_ptr and bitwidth_ptr == ano.bitwidth_ptr;
}

bool ExecInfo::operator!=(const ExecInfo &ano) const {
    return pc != ano.pc;
}

ExecInfo::ExecInfo(const ExecInfo &ano) = default;





