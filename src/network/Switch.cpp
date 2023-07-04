//
// Created by xyfuture on 2023/3/16.
//

#include <memory>
#include <utility>
#include "Switch.h"
#include "network/SwitchSocket.h"

Switch::Switch(const sc_module_name &name, int switch_id):
sc_module(name),switch_id(switch_id),socket(nullptr),network(nullptr){
    SC_THREAD(processTransport);
}


void Switch::processTransport() {
    while (true) {
        if (pending_queue.empty()) {
            wait(trigger);
        }

        // queue not empty
        auto info = pending_queue.front();
        auto trans = info.first;
        auto is_send = info.second; // is send mode or not
        pending_queue.pop();

        auto src = trans->src;
        auto dst = trans->dst;
        auto request_data_size = trans->request_data_size;

        auto send_latency = network->transfer(src, dst, request_data_size); // get noc latency
        wait(send_latency, sc_core::SC_NS);

        auto target_switch = network->getSwitch(dst); // get dst switch

        sc_time delay(0, SC_NS);

        target_switch->receiveHandler(trans, delay);// call dst switch receive handler
        if (is_send){
            // send mode
            wait(delay); // wait until send success
            continue; // back to while
        }
        else {
            // get network latency of response data
            auto receive_latency = network->transfer(dst, src, trans->response_data_size);
            delay = delay + sc_time(receive_latency, SC_NS);
            wait(delay);
            // finish transport, call switch socket
            socket->transportFinish();
        }
    }
}

void Switch::registerSwitchSocket(SwitchSocket * _socket) {
    socket = _socket;
}

void Switch::transportHandler(std::shared_ptr<NetworkPayload> trans) {
    pending_queue.emplace(trans, false);
    trigger.notify();
}

void Switch::receiveHandler(std::shared_ptr<NetworkPayload> trans, sc_time & delay) {
    socket->receiveHandler(std::move(trans),delay);
}

void Switch::bind(Network * _network) {
    network = _network;
    network->registerSwitch(switch_id,this);
}

void Switch::sendHandler(std::shared_ptr<NetworkPayload> send_request) {
    pending_queue.emplace(send_request,true);
    trigger.notify();
}



