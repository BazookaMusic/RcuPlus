// Copyright 2019 Sotiris Dragonas
#include "include/urcu.hpp"

// register a thread to the rcu service
// and return an object to create read locks
// and synchronize with readers
// id should be unique and between 0 and thread_num
RCUSentinel RCU::urcu_register_thread(int thread_id) {
    assert(thread_id >= 0 && thread_id < this->threads);
    return RCUSentinel(thread_id, this);
}
