//
// Created by 17280 on 2023/9/10.
//

#ifndef PIMSIM_NN_PERFORMANCECOUNTER_H
#define PIMSIM_NN_PERFORMANCECOUNTER_H

#include <unordered_map>
#include <utility>
#include "nlohmann/json.hpp"
#include "isa/Instruction.h"
#include <systemc>


using ull = unsigned long long;

class InstructionTrace{
public:
    InstructionTrace(std::string op,std::string  type, ull start,ull end):
        op(std::move(op)),type(std::move(type)),start(start),end(end) { }
    std::string op ;
    std::string type ;
    ull start ;
    ull end;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(InstructionTrace,op,type,start,end);
};



class PerformanceCounter {

public:
    explicit PerformanceCounter(int core_id);

    void startRecord(const std::shared_ptr<Instruction>& inst, ull start_time);
    void startRecord(const std::shared_ptr<Instruction>& inst);
    void finishRecord(const std::shared_ptr<Instruction>& inst, ull end_time);
    void finishRecord(const std::shared_ptr<Instruction>& inst);



    std::map<std::string,double> getWeightedStatistics();
    std::map<std::string,double> getStatistics();

    std::vector<InstructionTrace>& getInstTraces();
    nlohmann::json getInstTracesJson();



public:
    const int core_id;

private:
    std::vector<InstructionTrace> inst_traces;
    std::unordered_map<std::shared_ptr<Instruction>,ull> inst_map;

};


#endif //PIMSIM_NN_PERFORMANCECOUNTER_H
