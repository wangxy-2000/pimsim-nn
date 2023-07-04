//
// Created by xyfuture on 2023/5/2.
//



#ifndef UTILS_TIMER_H_
#define UTILS_TIMER_H_

#include <systemc>
#include <functional>

using namespace sc_core;

class Timer:sc_module {
    SC_HAS_PROCESS(Timer);

public:
    Timer(const sc_module_name& name,double ,sc_time_unit unit);
    Timer(const sc_module_name& name,double interval,sc_time_unit unit,const std::function<void()>& handler);


    void process ();

    static void defaultHandler();

private:
    double interval;
    sc_time_unit unit;

    std::function<void()> handler;

};

class ProgressBar{
public:
    ProgressBar(sc_time_unit unit,int levels,double total_time,std::function<void(int)> handler_);


    void process();
    void defaultHandler(int progress) const;

private:
    Timer timer;
    std::function<void(int)> handler;

    double total_time;
    int levels;
    int cur_level;

};



#endif //UTILS_TIMER_H_
