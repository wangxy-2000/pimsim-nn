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
: BaseCoreModule(name,config,sim_config,core_ptr,clk), pulse_commit("pulse",clk),
  memory_socket("memory_socket"),
  vector_fsm_reg("vector_fsm", clk)
{
    // bind fsm register and input/output wire
    vector_fsm_reg.input.bind(vector_fsm_in);
    vector_fsm_reg.in_ready.bind(vector_ready_port);

    pulse_commit.out_port.bind(vector_commit_port);

    SC_METHOD(me_checkVectorInst);
    sensitive << vector_port;


    SC_THREAD(process);

    energy_counter.setStaticPowerMW(config.vector_reg_static_power+config.vector_alu_static_power);
}

void VectorUnit::process() {
    while (true) {
        // wait for new vector inst
        wait(vector_fsm_reg.start_exec);


        // stall pipeline, FSM will auto set ready to false
        // vector_ready_port.write(false);

        const auto& exec_info = vector_fsm_reg.read();
        auto vector_info = vector_fsm_reg.read().getVectorInfo();
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

        // restore pipeline  FSM will auto set to true
        // vector_busy_port.write(false);

        // reset fsm
        // vector_fsm_reg.finish_exec.notify(SC_ZERO_TIME);
        //  commit to rob
        vector_fsm_reg.finishExec([this,exec_info]{pulse_commit.write(exec_info);});


    }
}

void VectorUnit::me_checkVectorInst() {
    const auto& cur_info  = vector_port.read();

    // if current inst is vector inst then mark fsm input as valid
    if (cur_info.valid and cur_info.payload.exec_unit == +ExecType::Vector)
        vector_fsm_in.write({cur_info.payload,true});
    else
        vector_fsm_in.write({ExecInfo(),false});

}

int VectorUnit::getVectorComputeLatencyCyclePower(const VectorInfo & vector_info) {
    auto len = vector_info.len;

    int times = ceil(len * 1.0 / core_config.vector_width);


    double alu_energy = core_config.vector_alu_dynamic_energy * times;
    double reg_energy = (core_config.vector_reg_read_dynamic_energy*2+core_config.vector_reg_write_dynamic_energy)* times;

    energy_counter.addDynamicEnergyPJ(0,sc_time_stamp(),alu_energy+reg_energy);

    return times * core_config.vector_latency_cycle;

}

std::string VectorUnit::getStatus() {
    std::stringstream  s;

    s<<"Core:"<<core_ptr->getCoreID()<<" Vector>"<<" time: "<<sc_time_stamp()<<"\n";

    const auto& vector_info = vector_fsm_reg.read().getVectorInfo();

    s<<"pc:"<<vector_info.pc<<" op:"<<vector_info.op._to_string()<<"\n\n";

    return s.str();
}





