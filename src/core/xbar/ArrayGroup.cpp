//
// Created by xyfuture on 2023/8/17.
//

#include "ArrayGroup.h"
#include "core/Core.h"

CommitQueue::CommitQueue(const sc_module_name &name,ClockDomain* clk) :
sc_module(name),clk(clk) {
    SC_METHOD(me_processCommit);
    sensitive<<trigger;
}

void CommitQueue::me_processCommit() {
    if (not q.empty()){
        auto exec_info = q.front();
        out_port.write(exec_info);
        q.pop();
        clk->notifyNextPosEdge(&trigger);
    } else {
        out_port.write(ExecInfo());
    }
}

void CommitQueue::commit(const ExecInfo &exec_info) {
    q.push(exec_info);
    clk->notifyNextPosEdge(&trigger);
}

ArrayGroup::ArrayGroup(const sc_module_name &name, const MatrixUnitConfig &matrix_config_, ClockDomain* clk_,
                       int xbar_count,int array_group_id,EnergyCounter* counter,CommitQueue* commit_queue):
sc_module(name),fsm("FSM",clk_), memory_socket("socket"), xbar_array(matrix_config_),energy_counter(counter),
xbar_count(xbar_count),array_group_id(array_group_id), matrix_config(matrix_config_),commit_queue(commit_queue)
{
    fsm.input.bind(fsm_in);
    fsm.in_ready.bind(in_ready); // TODO check

    SC_THREAD(th_processMVM);
}

void ArrayGroup::th_processMVM() {
    while (true){
        wait(fsm.start_exec);


        const auto& exec_info = fsm.read();
        auto matrix_info = exec_info.getMatrixInfo();
        const auto& op = matrix_info.op;
        // only this one
        if (op != +Opcode::mvmul)
            return;

        auto input_bit = matrix_info.input_bitwidth;
        auto output_bit = matrix_info.output_bitwidth;

        auto input_byte = int(ceil(input_bit /8.0));;
        auto output_byte = int(ceil(output_bit /8.0));;

        auto read_addr = matrix_info.rs1_value;
        auto write_addr = matrix_info.rd_value;

        auto xbar_weight_shape = xbar_array.getWeightShape(matrix_info.mbiw);

        int read_size = xbar_weight_shape.first * input_byte;
        int write_size = xbar_weight_shape.second * output_byte * xbar_count;

        // for compare with mnsim
        read_size = read_size * 8;

        auto read_trans = TransactionPayload{
                .command=TransCommand::read,.addr=read_addr,.data_size=read_size,.data_ptr=nullptr
        };

        auto write_trans = TransactionPayload{
                .command=TransCommand::write,.addr=write_addr,.data_size=write_size,.data_ptr=nullptr
        };

        sc_time delay (0,sc_core::SC_NS);

        // read vector
        memory_socket.transport(read_trans,delay);

        // compute
        auto compute_latency_cycle = getArrayGroupComputeLatencyEnergy(matrix_info);
        auto compute_latency = compute_latency_cycle * matrix_config.period;

        // read and compute delay
        delay += sc_time(compute_latency,SC_NS);
        wait(delay);

        // write back result
        delay = sc_time(0,SC_NS);
        memory_socket.transport(write_trans,delay);
        wait(delay);

        // finish execute
        fsm.finishExec();
        commit_queue->commit(exec_info);

    }
}

int ArrayGroup::getArrayGroupComputeLatencyEnergy(const MatrixInfo &matrix_info) {
    int weight_precision = matrix_info.mbiw;
    int input_precision = matrix_info.input_bitwidth;
    int output_precision = matrix_info.output_bitwidth;


    auto latency_energy = xbar_array.getXbarArrayLatencyEnergy(weight_precision,
                                                               input_precision,
                                                               output_precision);

    energy_counter->addDynamicEnergyPJ(latency_energy.second*xbar_count);

    return latency_energy.first;
}

void ArrayGroup::start_of_simulation() {
    in_ready.write(true);

//    sc_module::start_of_simulation();
}

std::string ArrayGroup::getStatus() {
    stringstream  s;

    s<<"Array Group ID:"<<array_group_id<<" time: "<<sc_time_stamp()<<"\n";

    const auto& exec_info = fsm.read();

    s<<exec_info;

    return s.str();
}


void AGDispatcher::me_dispatch() {

    const auto& vp = reg_out.read();

    if (vp.valid){
        auto exec_info = vp.payload;
        auto matrix_info = exec_info.getMatrixInfo();
        auto ag_id = matrix_info.group;


        for(int i=0;i<out_n;i++){
            if (i == ag_id)
                data_out_array[i].write(vp);
            else
                data_out_array[i].write({ExecInfo(), false});
        }

        if (out_ready_array[ag_id].read()){ // out ready, dispatch
            in_ready.write(true);
            reg_enable.write(true);
        } else { // out not ready
            in_ready.write(false);
            reg_enable.write(false);
        }

    } else { // write invalid to all out ports
        in_ready.write(true);
        reg_enable.write(true);
        for(int i=0;i<out_n;i++){
            data_out_array[i].write({ExecInfo(), false});
        }
    }

}

AGDispatcher::AGDispatcher(const sc_module_name &name, int out_n_,ClockDomain* clk):
sc_module(name),out_n(out_n_), reg("reg",clk),
data_out_array(out_n_),out_ready_array(out_n_){
    reg.input.bind(data_in);
    reg.output.bind(reg_out);
    reg.enable.bind(reg_enable);

    SC_METHOD(me_dispatch);
    sensitive<<reg_out;
    for(const auto& out_ready:out_ready_array){
        sensitive<<out_ready;
    }

}
