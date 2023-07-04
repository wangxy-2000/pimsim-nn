//
// Created by xyfuture on 2023/6/14.
//

#include "ClockDomain.h"

ClockDomain::ClockDomain(const sc_module_name &name,double period ) : sc_module(name),period(period) {
    SC_THREAD(process);

    SC_METHOD(endPosEdge);
    sensitive<<end_pos_edge;

}

// update clock
void ClockDomain::process() {
    while (true){
        wait(period,sc_core::SC_NS);

        // at pos edge

        is_pos_edge = true; // for function bool posedge()

        for(const auto& event_ptr:pos_edge_events){
            event_ptr->notify(); // immediate notify
        }

        pos_edge_events.clear();


        end_pos_edge.notify(SC_ZERO_TIME); // next delta cycle
        // pos edge end


        // Not Implement Yet

        // pos level
        // neg edge
        // neg level
    }
}

void ClockDomain::notifyNextPosEdge(sc_event *event_ptr) {
    pos_edge_events.insert(event_ptr);
}

bool ClockDomain::posedge() const {
    return is_pos_edge;
}

void ClockDomain::endPosEdge() {
    is_pos_edge = false;
}
