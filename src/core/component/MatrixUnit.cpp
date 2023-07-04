//
// Created by xyfuture on 2023/3/30.
//

#include "MatrixUnit.h"
#include "core/Core.h"
#include <sstream>



MatrixUnit::MatrixUnit(const sc_module_name &name, const CoreConfig &config,const SimConfig& sim_config,Core* core_ptr,ClockDomain* clk)
: BaseCoreModule(name,config,sim_config,core_ptr,clk),
 memory_socket("memory_socket"),
  xbar_array(config.matrix_config),
  matrix_fsm("matrix_fsm",clk){

    matrix_fsm.input.bind(matrix_fsm_in);
    matrix_fsm.output.bind(matrix_fsm_out);

    SC_THREAD(process);

    SC_METHOD(checkMatrixInst);
    sensitive<<id_matrix_port;

    energy_counter.setStaticPowerMW(xbar_array.getStaticPower()*config.matrix_config.xbar_array_count);

}

void MatrixUnit::checkMatrixInst() {
    auto matrix_info = id_matrix_port.read();

    if (is_payload_valid(matrix_info))
         matrix_fsm_in.write({matrix_info,true});
    else
        matrix_fsm_in.write({matrix_info,false});

}

void MatrixUnit::process() {
    while(true) {
        // wait for matrix inst
        wait(matrix_fsm.start_exec);

        // stall pipeline
        matrix_busy_port.write(true);

        auto matrix_info = matrix_fsm_out.read();
        auto op = matrix_info.op; // only one op

        // only this one
        if (op != +Opcode::mvmul)
            return;

        auto input_bit = matrix_info.input_bitwidth;
        auto output_bit = matrix_info.output_bitwidth;

        auto input_byte = int(ceil(input_bit /8.0));;
        auto output_byte = int(ceil(output_bit /8.0));;

        auto read_addr = matrix_info.rs1_value;
        auto write_addr = matrix_info.rd_value;

        auto mbiw = matrix_info.mbiw;
        auto group = matrix_info.group;  // currently group means use how many xbar arrays
        // in the future this will be changed to which group

        auto xbar_weight_shape = xbar_array.getWeightShape(mbiw);

        int read_size = xbar_weight_shape.first * input_byte;
        int write_size = xbar_weight_shape.second * group * output_byte;

        auto read_trans = TransactionPayload{
            .command=TransCommand::read,.addr=read_addr,.data_size=read_size,.data_ptr=nullptr
        };

        auto write_trans = TransactionPayload{
            .command=TransCommand::write,.addr=write_addr,.data_size=write_size,.data_ptr=nullptr
        };

        sc_time delay (0,sc_core::SC_NS);

        // read vector
        memory_socket.transport(read_trans,delay);

        auto compute_latency_cycle = getMatrixComputeLatencyCyclePower(matrix_info);
        auto compute_latency = compute_latency_cycle * core_config.matrix_config.period;

        delay += sc_time(compute_latency,SC_NS);
        wait(delay);

        delay = sc_time(0,SC_NS);

        // write back result
        memory_socket.transport(write_trans,delay);
        wait(delay);

        // finish exec

        // restore pipeline
        matrix_busy_port.write(false);
        // reset fsm
        matrix_fsm.finish_exec.notify(SC_ZERO_TIME);

        if (sim_config.sim_mode == 1)
            if (isEndPC(matrix_info.pc))
                core_ptr->setFinish();
    }
}

int MatrixUnit::getMatrixComputeLatencyCyclePower(const MatrixInfo & matrix_info) {

    int weight_precision = matrix_info.mbiw;
    int input_precision = matrix_info.input_bitwidth;
    int output_precision = matrix_info.output_bitwidth;

    int count = matrix_info.group;

    auto latency_energy = xbar_array.getXbarArrayLatencyEnergy(weight_precision,
                                                               input_precision,
                                                               output_precision);

    energy_counter.addDynamicEnergyPJ(latency_energy.second*count);

    return latency_energy.first;
}

std::string MatrixUnit::getStatus() {
    std::stringstream s;

    auto matrix_info = matrix_fsm_out.read();

    s<<"Matrix>"<<" time:"<<sc_time_stamp().to_string()<<"\n"
    <<"pc:"<<matrix_info.pc<<" op:"<<matrix_info.op._to_string()<<"\n\n";

    return s.str();
}


