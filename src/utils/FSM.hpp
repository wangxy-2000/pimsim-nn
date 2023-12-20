//
// Created by xyfuture on 2023/6/14.
//

#ifndef UTILS_FSM_H_
#define UTILS_FSM_H_

#include <iostream>
#include <systemc>
#include "utils/ClockDomain.h"
#include "utils/ValidPayload.h"
#include <functional>


using namespace sc_core;


template <typename T>
class FSM:public sc_module{
    SC_HAS_PROCESS(FSM);
public:
    FSM(const sc_module_name& name,ClockDomain* clk,const std::function<void(void)>& callback= nullptr):
    sc_module(name),clk(clk),finish_callback(nullptr){
        SC_METHOD(process);
        sensitive<< trigger;

        SC_METHOD(processInput);
        sensitive<< input;

        SC_METHOD(_finishExec);
        sensitive<< finish_exec;

        SC_METHOD(setInReady);
        sensitive<<setReady;

    }

    void process(){
         if(is_ready and clk->posedge()){
             const auto& tmp_payload = input.read();
             if (tmp_payload.valid){
                 // start to exec
                 value = tmp_payload.payload;
                 is_ready = false;
                 setReady.notify();
                 start_exec.notify();
             }
         }
    }

    void setInReady(){
        in_ready.write(is_ready);
    }

    void processInput(){
        if (input.read().valid){
            clk->notifyNextPosEdge(&trigger);
        }
    }


    const T& read(){
        return value;
    }

    void finishExec(){
        clk->notifyNextPosEdge(&finish_exec);
    }

    void finishExec(const std::function<void(void)>& callback){
        finish_callback = callback;
        finishExec();
    }

    void setFinishCallback(const std::function<void(void)>& callback){
        finish_callback = callback;
    }

private:
    void _finishExec(){
        // check for next cycle waiting input
        clk->notifyNextPosEdge(&trigger);

        is_ready = true;
        setReady.notify(SC_ZERO_TIME);
        if (finish_callback)
            finish_callback();
    }


private:
    bool is_ready = true;
    T value;
    std::function<void(void)>  finish_callback;


private:
    ClockDomain* clk;
    sc_event trigger ;
    sc_event setReady;
    sc_event finish_exec;

public:
    sc_event start_exec;
    sc_in<ValidPayload<T>> input ;
    sc_out<bool> in_ready;

};



#endif //UTILS_FSM_H_
