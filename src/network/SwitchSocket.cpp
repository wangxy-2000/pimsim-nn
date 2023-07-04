//
// Created by xyfuture on 2023/3/22.
//

#include "SwitchSocket.h"

#include <utility>

SwitchSocket::SwitchSocket(std::string name, sc_event *event):
        socket_name(std::move(name)), event_ptr(event), switch_(nullptr){

}

SwitchSocket::SwitchSocket(std::string name):
        socket_name(std::move(name)), event_ptr(nullptr), switch_(nullptr){

}

void SwitchSocket::transportFinish() {
    event_ptr->notify();
}

void SwitchSocket::receiveHandler(std::shared_ptr<NetworkPayload> trans, sc_time & delay) {
    receive_handler(std::move(trans),delay);
}

void SwitchSocket::registerReceiveHandler(const std::function<void(std::shared_ptr<NetworkPayload>, sc_time &)>& handler) {
    receive_handler = handler;
}

void SwitchSocket::bind(Switch * _switch) {
    switch_ = _switch;
    switch_->registerSwitchSocket(this);
}

sc_event* SwitchSocket::transport(std::shared_ptr<NetworkPayload> trans) {
    // only set event_ptr can use this mode
    if (event_ptr == nullptr)
        throw "Not set event ptr";

    switch_->transportHandler(std::move(trans));

    return event_ptr;
}

std::string SwitchSocket::getSocketName() {
    return socket_name;
}

void SwitchSocket::send(std::shared_ptr<NetworkPayload> send_request) {
    switch_->sendHandler(std::move(send_request));
}


