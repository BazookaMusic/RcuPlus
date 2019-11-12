// Copyright 2019 Sotiris Dragonas
#include <iostream>
#include "include/urcu.hpp"

// RCULock
URCU::RCULock::RCULock(const int i, RCUNode** rcu_table, const int threads):
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
URCU::RCU::RCU(int num_threads) : threads(num_threads), curr_thread_index(-1) {
    rcu_table = new RCUNode*[threads];

    // make sure there's no line sharing
    RCUNode* new_node;
    for (int i = 0; i < threads; i++) {
        new_node = new RCUNode;
        new_node->time = 1;
        rcu_table[i] = new_node;
    }
}

URCU::RCU::~RCU() {
    for (int i = 0; i < threads; i++) {
        delete rcu_table[i];
    }

    delete [] rcu_table;
}

URCU::RCUSentinel URCU::RCU::urcu_register_thread() {
    ++curr_thread_index;
    return RCUSentinel(curr_thread_index, this);
}


// RCUSentinel
URCU::RCUSentinel::RCUSentinel(const int id, RCU* _rcu): index(id),
    rcu(_rcu), times(nullptr) {
    assert(rcu);
    }

URCU::RCUSentinel::~RCUSentinel() {
    if (times) {
        delete [] times;
    }
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

    if (!times) {
        times = new int64_t[rcu->curr_thread_index + 1];
    }
    for (int i = 0; i < rcu->curr_thread_index + 1; i++) {
        times[i] =
            rcu->rcu_table[i]->time.load(std::memory_order_relaxed);
    }

    for (int i = 0; i < rcu->curr_thread_index + 1; i++) {
        if (times[i] & 1) continue;
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

