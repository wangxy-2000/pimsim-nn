//
// Created by xyfuture on 2023/3/15.
//

#include "VectorUnit.h"
#include "core/Core.h"
#include <sstream>

const std::set<Opcode> VectorUnit::inst_read_double_vector = {
        Opcode::vvadd,Opcode::vvsub,Opcode::vvmul,Opcode::vvdmul,
        Opcode::vvmax,Opcode::vvsll,Opcode::vvsra
};

const std::set<Opcode> VectorUnit::inst_read_single_vector = {
        Opcode::vavg,Opcode::vrelu,Opcode::vtanh,Opcode::vsigm,
        Opcode::vmv,Opcode::vrsu,Opcode::vrsl
};

const std::set<Opcode> VectorUnit::inst_use_output_byte = {
        Opcode::vvmul,Opcode::vvsll,Opcode::vvsra,Opcode::vtanh,Opcode::vsigm,
        Opcode::vrsu,Opcode::vrsl
};


VectorUnit::VectorUnit(const sc_module_name &name, const CoreConfig &config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk)
: BaseCoreModule(name,config,sim_config,core_ptr,clk),
  memory_socket("memory_socket"),
  vector_fsm_reg("vector_fsm", clk)
{
    // bind fsm register and input/output wire
    vector_fsm_reg.input.bind(vector_fsm_in);
    vector_fsm_reg.output.bind(vector_fsm_out);

    SC_METHOD(checkVectorInst);
    sensitive<<id_vector_port;

    SC_THREAD(process);

    energy_counter.setStaticPowerMW(config.vector_reg_static_power+config.vector_alu_static_power);
}

void VectorUnit::process() {
    while (true) {
        // wait for new vector inst
        wait(vector_fsm_reg.start_exec);


        // stall pipeline
        vector_busy_port.write(true);

        auto vector_info = vector_fsm_out.read();
        auto op = vector_info.op;

        // confirm data width
        auto input_byte = int(ceil(vector_info.input_bitwidth / 8.0));
        auto output_byte = int(ceil(vector_info.output_bitwidth / 8.0));

        // compute per vector data size read from local memory
        int req_data_size = input_byte * vector_info.len;

        // written vector size
        int wb_data_size = input_byte * vector_info.len; // default
        if (inst_use_output_byte.find(op) != inst_use_output_byte.end()) {
            wb_data_size = output_byte * vector_info.len;
            if (op == +Opcode::vvdmul || op == +Opcode::vavg)
                wb_data_size = output_byte;
        }


        if (inst_read_double_vector.find(op) != inst_read_double_vector.end()) {
            // inst need two operands

            TransactionPayload trans_src1 = {.command=TransCommand::read, .addr=vector_info.rs1_value,
                    .data_size=req_data_size, .data_ptr=nullptr};
            TransactionPayload trans_src2 = {.command=TransCommand::read, .addr=vector_info.rs2_value,
                    .data_size=req_data_size, .data_ptr=nullptr};

            sc_time delay(0, sc_core::SC_NS);
            memory_socket.transport(trans_src1, delay);
            memory_socket.transport(trans_src2, delay);


            auto compute_latency = getVectorComputeLatencyCyclePower(vector_info);
            delay += sc_time(compute_latency, SC_NS);

            wait(delay);

            delay = sc_time(0, SC_NS);

            // finish computing , write back to local memory

            TransactionPayload trans_dst = {.command=TransCommand::write, .addr=vector_info.rd_value,
                    .data_size=wb_data_size, .data_ptr=nullptr};
            memory_socket.transport(trans_dst, delay);
            wait(delay);
        }
        else {
            // inst only need one operand
            // TODO 关于VMV指令
            TransactionPayload trans_src = {.command=TransCommand::read, .addr=vector_info.rs1_value,
                    .data_size=req_data_size, .data_ptr=nullptr};

            sc_time delay(0, SC_NS);

            memory_socket.transport(trans_src, delay);

            auto compute_latency = getVectorComputeLatencyCyclePower(vector_info);
            delay += sc_time(compute_latency, SC_NS);

            wait(delay);

            delay = sc_time(0, SC_NS);

            TransactionPayload trans_dst = {.command=TransCommand::write, .addr=vector_info.rd_value,
                    .data_size=wb_data_size, .data_ptr=nullptr};
            memory_socket.transport(trans_dst, delay);
            wait(delay);
        }

        // exec finish

        // restore pipeline
        vector_busy_port.write(false);
        // reset fsm
        vector_fsm_reg.finish_exec.notify(SC_ZERO_TIME);

        if (sim_config.sim_mode == 1)
            if (isEndPC(vector_info.pc))
                core_ptr->setFinish();


    }
}

void VectorUnit::checkVectorInst() {
    auto vector_info = id_vector_port.read();

    // if current inst is vector inst then mark fsm input as valid
    if(is_payload_valid(vector_info))
        vector_fsm_in.write({vector_info, true});
    else
        vector_fsm_in.write({vector_info, false});

}

int VectorUnit::getVectorComputeLatencyCyclePower(const VectorInfo & vector_info) {
    auto len = vector_info.len;

    int times = ceil(len * 1.0 / core_config.vector_width);


    double alu_energy = core_config.period * core_config.vector_latency_cycle * times;
    double reg_energy = core_config.period * core_config.vector_reg_dynamic_power* times;
    energy_counter.addDynamicEnergyPJ(0,sc_time_stamp(),alu_energy+reg_energy);

    return times * core_config.vector_latency_cycle;

}

std::string VectorUnit::getStatus() {
    std::stringstream  s;

    auto vector_info = vector_fsm_out.read();

    s<<"Vector>"<<" time:"<<sc_time_stamp().to_string()<<"\n"
    <<"pc:"<<vector_info.pc<<" op:"<<vector_info.op._to_string()<<"\n\n";

    return s.str();
}


