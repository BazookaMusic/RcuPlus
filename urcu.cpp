// Copyright 2019 Sotiris Dragonas
#include "include/urcu.hpp"

// register a thread to the rcu service
// and return an object to create read locks
// and synchronize with readers
RCUSentinel RCU::urcu_register(int id) {
    return RCUSentinel(id, this);
}
