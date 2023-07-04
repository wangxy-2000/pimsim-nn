//
// Created by xyfuture on 2023/3/21.
//



#ifndef COMM_TARGETSOCKET_H_
#define COMM_TARGETSOCKET_H_

#include <systemc>
#include <functional>
#include "comm/TransactionPayload.hpp"

using namespace sc_core;

class InitiatorSocket;

// Target Socket
// receive request and process request
class TargetSocket {
public:
    explicit TargetSocket(std::string  name);

    void bind(InitiatorSocket*);

    void registerHandler(const std::function<void(TransactionPayload&,sc_time& )>& _handler);

    // once receive a request, call handler function to process
    // payload for request content
    // sc_time for process latency
    void Handler(TransactionPayload&,sc_time&);

    bool checkHandler();

    const std::string socket_name;
private:

    std::function<void(TransactionPayload&,sc_time& )> handler;
};


#endif //COMM_TARGETSOCKET_H_
