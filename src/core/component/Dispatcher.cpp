//
// Created by xyfuture on 2023/8/4.
//

#include "Dispatcher.h"
#include "core/Core.h"


Dispatcher::Dispatcher(const sc_module_name &name, const CoreConfig &config, const SimConfig &simConfig, Core *core_ptr,
                       ClockDomain *clk)
: BaseCoreModule(name, config, simConfig, core_ptr, clk),
  dispatcher_reg("reg",clk){
    dispatcher_reg.input.bind(id_dispatcher_port);
    dispatcher_reg.output.bind(dispatcher_reg_out);
    dispatcher_reg.in_ready.bind(dispatcher_ready_port);
    dispatcher_reg.out_ready.bind(self_ready);


    SC_METHOD(me_process);
    sensitive << dispatcher_reg_out << rob_ready << matrix_ready_port << vector_ready_port << transfer_ready_port << is_pend_inst_conflict;
}



bool Dispatcher::isDispatchable() {
    // stall info
    auto rob_ready_status = rob_ready.read();
    if (not rob_ready_status)
        return false;

    if (is_pend_inst_conflict.read())
        return false;

    const auto& cur_info = dispatcher_reg_out.read();
    if(cur_info.valid){
        auto& exec_info = cur_info.payload;
        if (exec_info.exec_unit == +ExecType::Matrix){
            return matrix_ready_port.read();
        } else if (exec_info.exec_unit == +ExecType::Vector){
            return vector_ready_port.read();
        } else if (exec_info.exec_unit == +ExecType::Transfer){
            return transfer_ready_port.read();
        }
    }

    return true;
}

void Dispatcher::me_process() {

    const auto& cur_info  = dispatcher_reg_out.read();

    pend_inst.write(cur_info.payload);


    if(isDispatchable()){
        dispatcher_out_port.write(cur_info);
        self_ready.write(true);
    } else {
        dispatcher_out_port.write({ExecInfo(),false});
        self_ready.write(false);
    }

}

std::string Dispatcher::getStatus() {
    std::stringstream  s;
    s <<"Core:"<<core_ptr->getCoreID()<<" Dispatch>"<<" time: "<<sc_time_stamp()<<"\n";

    auto cur_info = dispatcher_reg_out.read();

    if (cur_info.valid){
        auto dispatch_info = cur_info.payload;
        s<<dispatch_info;
    } else {
        s<<"invalid exec info\n";
    }

    return s.str();
}


