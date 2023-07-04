//
// Created by xyfuture on 2023/5/2.
//

#include "Timer.h"

Timer::Timer(const sc_module_name &name, double interval, sc_time_unit unit):
sc_module(name),interval(interval),unit(unit),handler(defaultHandler){
    SC_THREAD(process);
}

void Timer::process() {
    while(true){
        wait(interval,unit);
        handler();
    }
}

Timer::Timer(const sc_module_name &name, double interval, sc_time_unit unit, const std::function<void()>& handler):
sc_module(name),interval(interval),unit(unit),handler(handler){
    SC_THREAD(process);

}

void Timer::defaultHandler() {
    std::cout<<"Current Time: "<<sc_core::sc_time_stamp()<<std::endl;
}

ProgressBar::ProgressBar(sc_time_unit unit, int levels, double total_time, std::function<void(int)> handler_= nullptr):
        timer("Timer",total_time/levels,unit,[this](){ this->process();}),
        levels(levels),total_time(total_time),handler(handler_){
    cur_level = 1;
    if(!handler)
        handler = [this](int progress){ defaultHandler(progress);};
}

void ProgressBar::process() {
//    std::cout<<"Time:"<<sc_time_stamp()<<std::endl;
    handler(cur_level++);
}

void ProgressBar::defaultHandler(int progress) const {
    std::cout<<"Progress: "<<progress<<"/"<<levels<<std::endl;
}

