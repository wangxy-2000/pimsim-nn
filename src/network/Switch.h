//
// Created by xyfuture on 2023/3/16.
//



#ifndef NETWORK_SWITCH_H_
#define NETWORK_SWITCH_H_

#include <vector>
#include <queue>
#include <map>
#include <systemc>

#include "network/NetworkPayload.hpp"
#include "network/Network.h"

using namespace sc_core;

class SwitchSocket;


// Switch: used to send and recv data from network
// use SwitchSocket as bridge to connect Switch and Core Component
class Switch:public sc_module {
    SC_HAS_PROCESS(Switch);
public:
    explicit Switch(const sc_module_name& name,int switch_id);

    void processTransport();

    // two mode :
    // transport mode not only sends to dst,but also requires response from dst
    // send mode just sends data to dst without demands of response
    void transportHandler(std::shared_ptr<NetworkPayload>);
    void sendHandler(std::shared_ptr<NetworkPayload>);

    void receiveHandler(std::shared_ptr<NetworkPayload>, sc_time&);// when recv data from network,call this

    void registerSwitchSocket(SwitchSocket* );

    void bind(Network*);

private:
    sc_event trigger;

    SwitchSocket* socket;

    std::queue<std::pair<std::shared_ptr<NetworkPayload>,bool>> pending_queue;
    // true for no transport mode , false for send mode

    int switch_id;
    Network* network;

};


#endif //NETWORK_SWITCH_H_
