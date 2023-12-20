//
// Created by xyfuture on 2023/8/3.
//



#ifndef CORE_PAYLOADS_EXECINFO_H_
#define CORE_PAYLOADS_EXECINFO_H_

#include <utility>
#include <vector>
#include <memory>
#include "core/payloads/StagePayloads.hpp"
#include "better-enums/enum.h"


class Core;
typedef long long ll;

BETTER_ENUM(ExecType,int,Nop=1,Scalar,Matrix,Vector,Transfer);


template<typename T>
bool isOverlap(const std::pair<T,T>& a,const std::pair<T,T>& b){
    if (a.second<b.first || a.first>b.second)
        return false;
    return true;
}

class ExecInfo {
public:
    int pc;
    ExecType exec_unit;

    std::shared_ptr<Instruction> inst_ptr;
    std::shared_ptr<RegFileReadValue> reg_value_ptr;
    std::shared_ptr<BitwidthInfo> bitwidth_ptr;

    bool is_valid = false;
public:
    // local memory addr
    std::vector<std::pair<int,int>> memory_read_addr;
    std::vector<std::pair<int,int>> memory_write_addr;

    const Core* core_ptr;

public:
    ExecInfo();
    ExecInfo(int pc,
             const std::shared_ptr<Instruction>& inst,
             const std::shared_ptr<RegFileReadValue>& reg_value,
             const std::shared_ptr<BitwidthInfo>& bitwidth,
             Core* core_ptr);

    ExecInfo(const ExecInfo& ano); // copy constructor

    void parse(); // parse exec_unit and memory read write addr
    bool isValid() const ;
    bool isConflict(const ExecInfo& ano) const;

    bool operator==(const ExecInfo& ano) const;
    bool operator!=(const ExecInfo& ano) const;


private:
    int getFinalRS1() const;
    int getFinalRS2() const;
    int getFinalRD() const;

    ll getFinalRS1Double() const;
    ll getFinalRDDouble() const;

    void setMemoryReadWriteAddr();

public:
    MatrixInfo getMatrixInfo() const;
    VectorInfo getVectorInfo() const;
    TransferInfo getTransferInfo() const;
    ScalarInfo getScalarInfo() const;


public:
    friend std::ostream& operator << (std::ostream& out,const ExecInfo& info);
    friend void sc_trace(sc_core::sc_trace_file* f, const ExecInfo& self,const std::string& name);


};


#endif //CORE_PAYLOADS_EXECINFO_H_
