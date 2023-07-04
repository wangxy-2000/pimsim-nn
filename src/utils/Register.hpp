//
// Created by xyfuture on 2023/3/4.
//


#pragma once
#ifndef UTILS_REGISTER_H_
#define UTILS_REGISTER_H_

#include <systemc>
#include "utils/ClockDomain.h"
using namespace sc_core;



// discard
template<typename T>
class Register:public sc_module {
public:
    SC_HAS_PROCESS(Register);

    explicit Register(const sc_module_name& name):sc_module(name){
        SC_METHOD(update);
        sensitive<<clk.posedge_event();
    }

    void update(){
        if(clk.posedge()){
            value = input.read();
            output.write(value);
        }
    }

private:
    T value ;

    sc_in<T> input;
    sc_in<bool> clk; // use buffer instead signal as channel !
    sc_out<T> output;
};


// Used to model a Register with synchronization reset(next clock)
// Use ClockDomain as it's clock, no explicit clock (sc_clock)
// Only value change will trigger an update
template<typename T>
class RegNext:public sc_module{
public:
    SC_HAS_PROCESS(RegNext);

    RegNext(const sc_module_name& name,ClockDomain* clk_)
    :sc_module(name),clk(clk_){

        SC_METHOD(processInput);
        sensitive<<input;

        SC_METHOD(update);
        sensitive<<trigger;

    }

    void processInput(){
        // check if value change
        if (!(value == input.read()))
            clk->notifyNextPosEdge(&trigger); // update next pos edge
    }

    void update(){
        if (clk->posedge()) { // check pos edge
            value = input.read();

            if (enable_reset) {
                if (reset_ptr->read()){
                    value = reset_value;
                }
            }

            output.write(value);

        }
    }

    void setReset(const T& rst_value,const sc_signal<T>& rst_signal){
        enable_reset = true;

        reset_value = rst_value;
        reset_ptr = std::make_shared<sc_in<T>>();
        reset_ptr->bind(rst_signal);
    }

private:
    ClockDomain* clk;

    T value;
    sc_event trigger;

private:
    std::shared_ptr<sc_in<bool>> reset_ptr ;
    bool enable_reset = false;
    T reset_value ;

public:
    sc_in<T> input;
    sc_out<T> output;
};


// Used to model a register with enable wire and sync reset
template<typename T>
class RegEnable:public sc_module{
public:
    SC_HAS_PROCESS(RegEnable);

    RegEnable(const sc_module_name& name,ClockDomain* clk_)
    :sc_module(name),clk(clk_){
        SC_METHOD(processInput);
        sensitive<<input;
        SC_METHOD(processEnable);
        sensitive<<enable;

        SC_METHOD(update);
        sensitive<<trigger;

    }

    void processInput(){
        if (!(value == input.read()))
            clk->notifyNextPosEdge(&trigger);
    }

    void processEnable(){
        if (enable.read()) // enable is true, update next pos edge
            clk->notifyNextPosEdge(&trigger);
    }

    void update(){
        if (enable.read() and clk->posedge()){ // first check enable and clock
            value = input.read();

            if (enable_reset){
                if (reset_ptr->read()){
                    value = reset_value;
                }
            }

            output.write(value);

        }
    }

    // only when enable is true, reset the register
    void setReset(const T & rst_value,const sc_signal<bool>& rst_signal){
        enable_reset = true;

        reset_value = rst_value;
        reset_ptr = std::make_shared<sc_in<bool>>();
        reset_ptr->bind(rst_signal);
    }


private:
    ClockDomain* clk;

    T value;

    sc_event trigger;

private:
    std::shared_ptr<sc_in<bool>> reset_ptr;
    bool enable_reset = false;
    T reset_value;


public:
    sc_in<T> input;
    sc_in<bool> enable;
    sc_out<T> output;
};





#endif //UTILS_REGISTER_H_
