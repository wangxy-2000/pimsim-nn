//
// Created by xyfuture on 2023/6/14.
//



#ifndef UTILS_CLOCKDOMAIN_H_
#define UTILS_CLOCKDOMAIN_H_

#include <queue>
#include <set>
#include <systemc>

using namespace sc_core ;

// Model clock, cooperate with register
class ClockDomain:public sc_module {
    SC_HAS_PROCESS(ClockDomain);

public:
    explicit ClockDomain(const sc_module_name& name,double period);

    // update clock
    void process();

    // register use this function to trigger its update
    void notifyNextPosEdge(sc_event* event_ptr);

    // return is pos edge or not
    bool posedge() const;

private:
    void endPosEdge();


private:
    // when pos edge, notify these events immediately
    std::set<sc_event*> pos_edge_events ;
    std::set<sc_event*> neg_edge_events; // currently only positive edge work

private:
    sc_event end_pos_edge ;
    bool is_pos_edge = false;

private:
    // 1/frequency , unit -> ns
    // 1GHz -> 1 ns -> period = 1
    double period;
};


#endif //UTILS_CLOCKDOMAIN_H_
