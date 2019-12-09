// Copyright 2019 Sotiris Dragonas
#include <iostream>
#include "include/urcu.hpp"

// RCULock
URCU::RCULock::RCULock(const int i, URCU::RCU::RCUNode** rcu_table, const int threads):
             index(i), _rcu_table(rcu_table), threads(threads), valid(true) {
    // bound check
    assert(threads > 0 && index < threads && _rcu_table[index]);
    _rcu_table[index]->time += 1;
}

URCU::RCULock::~RCULock() {
            if (!valid) return;

            assert(threads > 0 && index < threads && _rcu_table[index]);
            _rcu_table[index]->time |= 1;
}

URCU::RCULock::RCULock(RCULock&& a_lock): index(a_lock.index),
        _rcu_table(a_lock._rcu_table),threads(a_lock.threads), valid(true) {
            a_lock.valid = false;
            a_lock._rcu_table = nullptr;
        }


// RCU
URCU::RCU::RCU(int num_threads) : threads(num_threads), curr_thread_index(-1), free_indices_stack(new int[num_threads]), stack_index(-1) {
    rcu_table = new URCU::RCU::RCUNode*[threads];

    // make sure there's no line sharing
    URCU::RCU::RCUNode* new_node;
    for (int i = 0; i < threads; i++) {
        new_node = new URCU::RCU::RCUNode;
        new_node->time = 1;
        rcu_table[i] = new_node;
    }
}

URCU::RCU::~RCU() {
    for (int i = 0; i < threads; i++) {
        delete rcu_table[i];
    }

    delete [] rcu_table;

    delete [] free_indices_stack;
}

URCU::RCUSentinel URCU::RCU::urcu_register_thread() {
    stack_lock.lock();
    static int index_to_ret = -1;
    
    if (stack_index >= 0) {
        index_to_ret = free_indices_stack[stack_index--];
    }
    else if (curr_thread_index < threads - 1) {
         ++curr_thread_index;
         index_to_ret = curr_thread_index;
    } else {
        stack_lock.unlock();
        std::cerr << "RCU: Not enough space to register threads" << std::endl;
        exit(-1);
    } 

    stack_lock.unlock();

    return RCUSentinel(index_to_ret, this);
}


// RCUSentinel
URCU::RCUSentinel::RCUSentinel(const int id, RCU* _rcu): index(id),
    rcu(_rcu), times(nullptr) {
    assert(rcu);
    rcu->rcu_table[index]->time = 1;
    }

URCU::RCUSentinel::~RCUSentinel() {
    if (times) {
        delete [] times;
    }

    rcu->rcu_table[index]->time = -1;

    rcu->stack_lock.lock();
    rcu->free_indices_stack[++rcu->stack_index] = index;
    rcu->stack_lock.unlock();
}

URCU::RCUSentinel::RCUSentinel(RCUSentinel&& a_sentinel): index(a_sentinel.index), 
            rcu(a_sentinel.rcu), times(a_sentinel.times) {
                a_sentinel.times = nullptr;
                a_sentinel.rcu = nullptr;
        }






void URCU::RCUSentinel::urcu_synchronize() {
    // micro optimization,
    // if sentinel never invokes synchronize,
    // no need to allocate heap memory
    if (!rcu) {
        return;
    }

    int curr_times_size = rcu->curr_thread_index;

    if (!times) {
        times = new int64_t[curr_times_size + 1];
    }
    for (int i = 0; i < curr_times_size + 1; i++) {
        times[i] =
            rcu->rcu_table[i]->time.load(std::memory_order_relaxed);
    }

    for (int i = 0; i < curr_times_size + 1; i++) {
        if (times[i] < 0 || times[i] & 1) continue;
        for (;;) {
            int64_t t =
                rcu->
                    rcu_table[i]->
                        time.load(std::memory_order_relaxed);

            if (t & 1 || t > times[i]) {
                break;
            }
        }
    }
}

