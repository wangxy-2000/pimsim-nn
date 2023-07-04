//
// Created by xyfuture on 2023/6/14.
//

#ifndef UTILS_FSM_H_
#define UTILS_FSM_H_

#include <iostream>
#include <systemc>
#include "utils/ClockDomain.h"

using namespace sc_core;


template <typename T>
struct FSMPayload{
    T  payload;
    bool valid;

    bool operator == (const FSMPayload& ano){
        return valid == ano.valid and payload == ano.payload;
    }

    friend void sc_trace(sc_core::sc_trace_file* f, const FSMPayload& self,const std::string& name){}

    friend std::ostream& operator<<(std::ostream& out,const FSMPayload & self ) {
        out << "FSMPayload Type\n";
        return out;
    }

};

template <typename T>
class FSM:public sc_module{
    SC_HAS_PROCESS(FSM);
public:
    FSM(const sc_module_name& name,ClockDomain* clk):
    sc_module(name),clk(clk){
        SC_METHOD(process);
        sensitive<< trigger;

        SC_METHOD(processInput);
        sensitive<< input;

        SC_METHOD(finishExec);
        sensitive<< finish_exec;
    }

    void process(){
         if(is_ready and clk->posedge()){
             auto tmp_payload = input.read();
             if (tmp_payload.valid){
                 // start to exec
                 value = tmp_payload.payload;
                 is_ready = false;
                 start_exec.notify(SC_ZERO_TIME);
                 output.write(value);
             }
         }
    }

    void processInput(){
        if (input.read().valid){
            clk->notifyNextPosEdge(&trigger);
        }
    }

    bool isReady(){
        return is_ready;
    }

    bool isBusy(){
        return !is_ready;
    }
private:
    void finishExec(){
        is_ready = true;
        clk->notifyNextPosEdge(&trigger);
    }

private:
    bool is_ready = true;
    T value;

private:
    ClockDomain* clk;
    sc_event trigger ;

public:
    sc_event finish_exec;

public:
    sc_event start_exec;

    sc_in<FSMPayload<T>> input ;
    sc_out<T> output;

};



#endif //UTILS_FSM_H_
