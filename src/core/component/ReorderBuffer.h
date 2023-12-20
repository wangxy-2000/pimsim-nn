//
// Created by xyfuture on 2023/8/4.
//



#ifndef CORE_COMPONENT_REORDERBUFFER_H_
#define CORE_COMPONENT_REORDERBUFFER_H_

#include <systemc>
#include "core/BaseCoreModule.h"
#include "core/payloads/ExecInfo.h"
#include "analysis/PerformanceCounter.h"
using namespace sc_core;


template <typename T>
class RingBufferIterator{
public:
    RingBufferIterator(std::pair<T,bool>* data,int array_capacity,int index):
            m_data(data),m_array_capacity(array_capacity),m_index(index){

    }

    RingBufferIterator& operator++ () {
        m_index = (m_index+1)%m_array_capacity;
        return *this;
    }

    RingBufferIterator& operator-- (){
        m_index = (m_index+m_array_capacity-1)%m_array_capacity;
        return *this;
    }

    std::pair<T,bool>& operator*() const {
        return m_data[m_index];
    }


    bool operator== (const RingBufferIterator& ano){
        return m_index == ano.m_index;
    }

    bool operator!=(const RingBufferIterator& ano){
        return m_index != ano.m_index;
    }


private:
    std::pair<T,bool>* m_data;
    int m_array_capacity;
    int m_index;
};


// designed for rob
template <typename T>
class RingBuffer{

public:
    explicit RingBuffer(int max_capacity):
    m_capacity(max_capacity),m_size(0),m_head(0),m_tail(0),m_array_capacity(max_capacity+1){
        m_data = new std::pair<T,bool> [m_array_capacity];
    }

    ~RingBuffer() {
        delete[] m_data;
    }

public:
    bool empty() const {
        return m_size == 0;
    }
    bool full() const{
        return m_size == m_capacity;
    }

    int size() const{
        return m_size;
    }

    void push(const T& value){
        if (full())
            throw std::out_of_range("ring buffer if full");
        m_data[m_tail] = std::make_pair(value,true);
        m_tail = (m_tail+1)%m_array_capacity;
        m_size++;
    }


    void remove(const T& value) { // only set to false
        // traverse to find
        auto search_head = m_head;
        auto search_size = 0;
        while(search_size<m_size){
            if (m_data[search_head].first == value){
                // find value

                m_data[search_head].second = false;
                retire(); // remove all false

                return;
            }
            search_head = (search_head+1)%m_array_capacity;
            search_size++;
        }
        throw "Can not find value";
    }

    void retire() { // clear all false element from back

        while(m_size > 0){
            if (!m_data[m_head].second){ // false
                m_head = (m_head+1)%m_array_capacity;
                m_size--;
            } else { // true
                return;
            }
        }

    }

    void retire(const T& value){// set to false and clear
        auto cur_head = m_head;
        int cur_size = 0;

        while(cur_size<m_size){
            if (m_data[cur_head].first == value){
                // find value
                m_data[cur_head].second = false;
                if (cur_head == m_head){
                    // search all false item
                    while(m_size > 0){

                        if(!m_data[m_head].second){
                            m_head = (m_head+1)%m_array_capacity;
                            m_size--;
                        }
                        else
                            return;

                    }
                }
                return;
            }
            cur_head = (cur_head+1)%m_array_capacity;
            cur_size++;
        }

        throw "Can not find value";

    }

    std::pair<bool,T> lastRetired(const T& value){
        auto cur_size = m_size;
        retire(value);
        auto new_size = m_size;
        if (cur_size!=new_size){
            auto last_head = (m_head+m_array_capacity-1) % m_array_capacity ;
            return {true,m_data[last_head].first};
        }
        return {false,T()};
    }


    const T& front(){// get front
        if (empty())
            throw std::out_of_range("ring buffer is empty");
        return m_data[m_head].first;
    }
    const T& back() {// get back
        if (empty())
            throw std::out_of_range("ring buffer is empty");
        return m_data[(m_tail+m_array_capacity-1)%m_array_capacity];
    }

    RingBufferIterator<T> begin(){
        return RingBufferIterator<T>(m_data,m_array_capacity,m_head);
    }

    RingBufferIterator<T> end(){
        return RingBufferIterator<T>(m_data,m_array_capacity,m_tail);
    }
private:
    std::pair<T,bool>* m_data;
    int m_size;
    int m_head;
    int m_tail;
    int m_capacity;
    int m_array_capacity; // capacity+1

};



class ReorderBuffer: public BaseCoreModule{
    SC_HAS_PROCESS(ReorderBuffer);
public:
    ReorderBuffer(const sc_module_name& name,int _capacity,const CoreConfig& config,const SimConfig& simConfig,Core* core_ptr,ClockDomain* clk);


    bool checkConflict(const ExecInfo& exec_info);

    std::string getStatus();

public:
    void me_processNewInst();
    void me_processCommitInst();
    void me_updateROB();

    void me_processPendInst();

    void me_checkBufferIsReady();


private:
    sc_event update_rob; // just update rob, may not change rob content
    sc_event rob_changed; // add or delete item in rob


private:
    RingBuffer<ExecInfo> ring_buffer;
    int capacity;

public:

    sc_in<ExecInfo> pend_inst; // inst need to be checked
    sc_out<bool> is_conflict;


    sc_in<VP<ExecInfo>> new_inst; // insert to ring buffer

    sc_in<ExecInfo> matrix_commit_inst; // delete from ring buffer
    sc_in<ExecInfo> vector_commit_inst;
    sc_in<ExecInfo> transfer_commit_inst;

    sc_out<bool> rob_ready; // ring buffer is full

public:
    PerformanceCounter perf_counter;
};


#endif //CORE_COMPONENT_REORDERBUFFER_H_
