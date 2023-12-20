//
// Created by xyfuture on 2023/8/16.
//



#ifndef UTILS_VALIDSIGNAL_H_
#define UTILS_VALIDSIGNAL_H_

#include <systemc>
#include "utils/ClockDomain.h"

using namespace sc_core;

template<typename T>
class PulsePort: public sc_module{
    SC_HAS_PROCESS(PulsePort);
public:
    PulsePort(const sc_module_name& name, ClockDomain* clk,const T& default_value_ = T())
    :sc_module(name),clk(clk),default_value(default_value_){
        SC_METHOD(me_writeValue);
        sensitive<<write_event<<reset_event;
    }


    void me_writeValue(){
        if (write_new_value){  // reset value
            out_port.write(value_to_write);
            write_new_value = false;
        } else {
            out_port.write(default_value);
        }
    }

    void write(const T& value){
        value_to_write = value;
        write_new_value = true;
        write_event.notify(SC_ZERO_TIME);
        clk->notifyNextPosEdge(&reset_event);
    }


    void setDefault_value(const T& value) {
        default_value = value;
    };


private:
    bool write_new_value;
    T default_value;
    T value_to_write;
    ClockDomain* clk;
    sc_event write_event;
    sc_event reset_event;


public:
    sc_out<T> out_port;
};


#endif //UTILS_VALIDSIGNAL_H_

