// Copyright 2019 Sotiris Dragonas
#include <iostream>
#include "include/urcu.hpp"

// RCULock
RCULock::RCULock(const int i, RCUNode** rcu_table, const int threads):
             index(i), _rcu_table(rcu_table), threads(threads), valid(true) {
    // bound check
    assert(threads > 0 && index < threads && _rcu_table[index]);
    _rcu_table[index]->time += 1;
}

RCULock::~RCULock() {
            if (!valid) return;

            assert(threads > 0 && index < threads && _rcu_table[index]);
            _rcu_table[index]->time |= 1;
}

RCULock::RCULock(RCULock&& a_lock): index(a_lock.index),
        _rcu_table(a_lock._rcu_table),threads(a_lock.threads), valid(true) {
            a_lock.valid = false;
            a_lock._rcu_table = nullptr;
        }


// RCU
RCU::RCU(int num_threads) : threads(num_threads) {
    rcu_table = new RCUNode*[threads];

    // make sure there's no line sharing
    alignas(URCU_CACHE_LINE) RCUNode* new_node;
    for (int i = 0; i < threads; i++) {
        new_node = new RCUNode;
        new_node->time = 1;
        rcu_table[i] = new_node;
    }
}

RCU::~RCU() {
    for (int i = 0; i < threads; i++) {
        delete rcu_table[i];
    }

    delete [] rcu_table;
}

RCUSentinel RCU::urcu_register_thread(int thread_id) {
    assert(thread_id >= 0 && thread_id < this->threads);
    return std::move(RCUSentinel(thread_id, this));
}


// RCUSentinel
RCUSentinel::RCUSentinel(const int id, RCU* _rcu): index(id),
    rcu(_rcu), times(nullptr) {
    assert(rcu);
    }

RCUSentinel::~RCUSentinel() {
    if (times) {
        delete [] times;
    }
}

 RCUSentinel::RCUSentinel(RCUSentinel&& a_sentinel): index(a_sentinel.index), 
            rcu(a_sentinel.rcu), times(a_sentinel.times) {
                a_sentinel.times = nullptr;
                a_sentinel.rcu = nullptr;
            }






void RCUSentinel::urcu_synchronize() {
    // micro optimization,
    // if sentinel never invokes synchronize,
    // no need to allocate heap memory
    if (!rcu) {
        return;
    }

    if (!times) {
        times = new int64_t[rcu->threads];
    }
    for (int i = 0; i < rcu->threads; i++) {
        times[i] =
            rcu->rcu_table[i]->time.load(std::memory_order_relaxed);
    }

    for (int i = 0; i < rcu->threads; i++) {
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

