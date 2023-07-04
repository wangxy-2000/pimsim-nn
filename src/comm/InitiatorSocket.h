//
// Created by xyfuture on 2023/3/21.
//



#ifndef COMM_INITIATORSOCKET_H_
#define COMM_INITIATORSOCKET_H_

#include <systemc>

#include <comm/TransactionPayload.hpp>

using namespace sc_core;

class TargetSocket;

// Initiator Socket
// Initiate a request and receive response
class InitiatorSocket {
public:
    explicit InitiatorSocket(std::string  name);

    void bind(TargetSocket*);

    // send request
    void transport(TransactionPayload& trans,sc_time& delay);

    const std::string socket_name;
private:
    TargetSocket* target_socket ;

};


#endif //COMM_INITIATORSOCKET_H_
