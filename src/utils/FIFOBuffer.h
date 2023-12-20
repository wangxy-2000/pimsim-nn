//
// Created by xyfuture on 2023/8/16.
//


#ifndef UTILS_FIFOBUFFER_H_
#define UTILS_FIFOBUFFER_H_

#include <systemc>
#include <queue>
#include "utils/ClockDomain.h"
#include "utils/ValidPayload.h"
using namespace sc_core;

template<typename T>
class FIFOBuffer:public sc_module{
    SC_HAS_PROCESS(FIFOBuffer);

public:
    FIFOBuffer(int max_capacity,ClockDomain* clk):sc_module(),max_capacity(max_capacity),clk(clk){
        SC_METHOD(me_process);
        sensitive<<process_trigger;
    }


    void me_process(){
        if(out_ready.read()){
            if (not q.empty())
                q.pop();
        }
        auto in_data = data_in.read();
        if (in_data.valid){
            if (q.size() < max_capacity)
                q.push(in_data.payload);
        }

        if (not q.empty()){
            auto front = q.front();
            data_out.write({front, true});
        } else
            data_out.write({T(), false});

        if (q.size() < max_capacity)
            in_ready.write(true);
        else
            in_ready.write(false);

        clk->notifyNextPosEdge(&process_trigger);

    }


public:
    sc_in<VP<T>> data_in ;
    sc_out<bool> in_ready ;

    sc_out<VP<T>> data_out ;
    sc_in<bool> out_ready ;

private:
    sc_event process_trigger;

    ClockDomain* clk;


private:
    std::queue<T> q;
    int max_capacity;

};


#endif //UTILS_FIFOBUFFER_H_
