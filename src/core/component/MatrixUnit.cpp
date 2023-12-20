//
// Created by xyfuture on 2023/3/30.
//

#include "MatrixUnit.h"
#include "core/Core.h"
#include <sstream>



MatrixUnit::MatrixUnit(const sc_module_name &name, const std::vector<int>& array_group_map_, const CoreConfig &config, const SimConfig& sim_config, Core* core_ptr, ClockDomain* clk)
: BaseCoreModule(name,config,sim_config,core_ptr,clk),
  ag_dispatcher("ag_dispatch", array_group_map_.size(), clk), commit_queue("queue", clk), xbar_group_map(array_group_map_),
  ag_ready_array(array_group_map_.size()), ag_dispatcher_out(array_group_map_.size())
{
    int array_group_cnt = xbar_group_map.size();

    ag_dispatcher.data_in.bind(ag_dispatcher_in);
    ag_dispatcher.in_ready.bind(matrix_ready_port);
    commit_queue.out_port.bind(matrix_commit_port);

    for(int i=0;i<array_group_cnt;i++){
        auto ag_name = std::string("ag")+std::to_string(i);
        std::unique_ptr<ArrayGroup> cur_ag_ptr ( new ArrayGroup(ag_name.c_str(), core_config.matrix_config, clk, array_group_map_[i], i, &energy_counter, &commit_queue));

        ag_dispatcher.data_out_array[i].bind(ag_dispatcher_out[i]);
        ag_dispatcher.out_ready_array[i].bind(ag_ready_array[i]);

        cur_ag_ptr->in_ready.bind(ag_ready_array[i]);
        cur_ag_ptr->fsm_in.bind(ag_dispatcher_out[i]);

        array_groups.push_back(std::move(cur_ag_ptr));

    }


    SC_METHOD(me_checkMatrixInst);
    sensitive<<matrix_port;

}





std::string MatrixUnit::getStatus() {
    std::stringstream s;

    const auto& cur_info =  matrix_port.read();

    if (cur_info.valid){
        auto exec_info = cur_info.payload;
        if (exec_info.exec_unit == +ExecType::Matrix){
            auto matrix_info = exec_info.getMatrixInfo();
            s<<"Matrix>"<<" time:"<<sc_time_stamp().to_string()<<"\n"
             <<"pc:"<<matrix_info.pc<<" op:"<<matrix_info.op._to_string()<<"\n\n";
        }

    }

    return s.str();
}

void MatrixUnit::me_checkMatrixInst() {
    const auto& cur_info = matrix_port.read();

    if (cur_info.valid and cur_info.payload.exec_unit == +ExecType::Matrix)
        ag_dispatcher_in.write(cur_info);
    else
        ag_dispatcher_in.write({ExecInfo(),false});

}

void MatrixUnit::bindMemorySocket(TargetSocket* target_socket) {
    for (auto& ag_ptr:array_groups){
        ag_ptr->memory_socket.bind(target_socket);
    }

}

