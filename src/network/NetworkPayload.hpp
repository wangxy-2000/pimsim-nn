//
// Created by xyfuture on 2023/3/22.
//

#pragma once
#ifndef NETWORK_NETWORKPAYLOAD_H_
#define NETWORK_NETWORKPAYLOAD_H_

#include <memory>

// NetworkPayload: Used by Network to transfer data
struct NetworkPayload{
    int src;
    int dst;

    // one payload contains request and its response(optional)
    int request_data_size;
    std::shared_ptr<void> request_payload;

    int response_data_size;
    std::shared_ptr<void> response_payload;

    template<typename T>
    std::shared_ptr<T> getRequestPayload(){
        return std::static_pointer_cast<T>(request_payload);
    }

    template<typename T>
    std::shared_ptr<T> getResponsePayload(){
        return std::static_pointer_cast<T>(response_payload);
    }
};


#endif //NETWORK_NETWORKPAYLOAD_H_
