//
// Created by xyfuture on 2023/3/22.
//



#ifndef NETWORK_SWITCHSOCKET_H_
#define NETWORK_SWITCHSOCKET_H_

#include <systemc>
#include <string>
#include <functional>
#include "network/Switch.h"
#include "network/NetworkPayload.hpp"

using namespace sc_core;

class SwitchSocket {
public:
    SwitchSocket(std::string name,sc_event* event);
    explicit SwitchSocket(std::string name); //this not support for transport mode

    std::string getSocketName();

    void bind(Switch* );

    // two mode:
    // transport mode sends data and requires response, block (use event_ptr)
    // send mode just sends data and non-block
    sc_event* transport(std::shared_ptr<NetworkPayload> );
    void send(std::shared_ptr<NetworkPayload>);

    // when receive data call receive handler
    void registerReceiveHandler( const std::function<void(std::shared_ptr<NetworkPayload>,sc_time&)>&);
    void receiveHandler(std::shared_ptr<NetworkPayload>,sc_time&);

    void transportFinish(); // event_ptr.notify()


private:
    const std::string socket_name;
    sc_event* event_ptr;

    Switch* switch_;

    std::function<void(std::shared_ptr<NetworkPayload> ,sc_time& delay)> receive_handler;

};


#endif //NETWORK_SWITCHSOCKET_H_
