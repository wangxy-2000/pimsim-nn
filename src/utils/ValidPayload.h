//
// Created by 17280 on 2023/8/16.
//

#ifndef PIMSIM_NN_VALIDPAYLOAD_H
#define PIMSIM_NN_VALIDPAYLOAD_H

#include "systemc"

using namespace sc_core;


template <typename T>
struct ValidPayload{
    ValidPayload() = default;
    ValidPayload(const T& payload,bool valid):payload(payload),valid(valid){}

    T  payload;
    bool valid = false;

    bool operator == (const ValidPayload& ano) const{
        return valid == ano.valid and payload == ano.payload;
    }

    friend void sc_trace(sc_core::sc_trace_file* f, const ValidPayload& self,const std::string& name){}

    friend std::ostream& operator<<(std::ostream& out,const ValidPayload & self ) {
        out << "ValidPayload Type\n";
        return out;
    }

};




template<typename T>
using VP = ValidPayload<T>;


#endif //PIMSIM_NN_VALIDPAYLOAD_H
