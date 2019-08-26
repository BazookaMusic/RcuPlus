// Copyright 2019 Sotiris Dragonas
#include "include/urcu.hpp"

// RCULock
RCULock::RCULock(const int i, RCUNode** rcu_table, const int threads):
             index(i), _rcu_table(rcu_table), threads(threads) {
    // bound check
    assert(threads > 0 && index < threads && _rcu_table[index]);
    _rcu_table[index]->time += 1;
}

RCULock::~RCULock() {
            assert(threads > 0 && index < threads && _rcu_table[index]);
            _rcu_table[index]->time |= 1;
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
    return RCUSentinel(thread_id, this);
}


// RCUSentinel
RCUSentinel::RCUSentinel(const int id, const RCU* _rcu): index(id),
    rcu(_rcu), times(nullptr) {
    assert(rcu);
    }

RCUSentinel::~RCUSentinel() {
    if (times) {
        delete [] times;
    }
}






void RCUSentinel::urcu_synchronize() {
    // micro optimization,
    // if sentinel never invokes synchronize,
    // no need to allocate heap memory
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

