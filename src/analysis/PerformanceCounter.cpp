//
// Created by 17280 on 2023/9/10.
//

#include "PerformanceCounter.h"
#include <queue>
#include <vector>
#include <iostream>

PerformanceCounter::PerformanceCounter(int core_id):
core_id(core_id){

}

void PerformanceCounter::startRecord(const std::shared_ptr<Instruction> &inst, ull start_time) {
    if (not inst_map.count(inst)){
        inst_map[inst] = start_time;
    } else
        throw "Performance Counter Repeated";

}

void PerformanceCounter::finishRecord(const std::shared_ptr<Instruction> &inst, ull end_time) {
    if (inst_map.count(inst)){
        auto start_time = inst_map[inst];
        auto op = inst->op._to_string();
        auto inst_type = inst->inst_type._to_string();

        inst_traces.emplace_back(op, inst_type, start_time, end_time);

        inst_map.erase(inst);

    } else
        throw "Performance Counter Not Found";

}

nlohmann::json PerformanceCounter::getInstTracesJson() {
    nlohmann::json tmp (inst_traces);
    return tmp;
}

std::map<std::string, double> PerformanceCounter::getWeightedStatistics() {
    struct start_cmp{
        bool operator () (const InstructionTrace &a, const InstructionTrace &b) {
            return a.start > b.start;
        }
    };

    auto end_cmp = [](const InstructionTrace &a, const InstructionTrace &b)->bool { return a.end > b.end;};

    std::priority_queue<InstructionTrace,std::vector<InstructionTrace>,start_cmp> start_queue (inst_traces.begin(), inst_traces.end());
//    std::priority_queue<InstructionTrace,std::vector<InstructionTrace>,end_cmp> end_queue;
    std::vector<InstructionTrace> end_queue;

    ull cur_time = 0 ;
    std::map<std::string,double> weighted_time;
    while ( not (start_queue.empty() and end_queue.empty()) ){
        while ( (not start_queue.empty()) and cur_time==start_queue.top().start ){
            const auto& top_trace = start_queue.top();

            end_queue.push_back(top_trace);
            std::push_heap(end_queue.begin(),end_queue.end(),end_cmp);

            start_queue.pop();
        }

        auto counts = end_queue.size();

        for(const auto& item: end_queue){
                weighted_time[item.op] += (1.0/counts);
        }

        while( (not end_queue.empty()) and cur_time == end_queue.front().end ){
            std::pop_heap(end_queue.begin(),end_queue.end(),end_cmp);
            end_queue.pop_back();
        }

        cur_time ++ ;
    }

    return weighted_time;
}

std::vector<InstructionTrace> &PerformanceCounter::getInstTraces() {
    return inst_traces;
}

std::map<std::string, double> PerformanceCounter::getStatistics() {
    std::map<std::string,double> time_breakdown;
    for(const auto& cur_inst : inst_traces){
        auto time = cur_inst.end - cur_inst.start;

        time_breakdown[cur_inst.op] += time;
    }

    return time_breakdown;
}

void PerformanceCounter::startRecord(const std::shared_ptr<Instruction> &inst) {
    auto current_time_ns = sc_core::sc_time_stamp().value()/1000;
    startRecord(inst,current_time_ns);
}

void PerformanceCounter::finishRecord(const std::shared_ptr<Instruction> &inst) {
    auto current_time_ns = sc_core::sc_time_stamp().value()/1000;
    finishRecord(inst,current_time_ns);
}






