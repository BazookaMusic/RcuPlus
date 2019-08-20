// Copyright 2019 Sotiris Dragonas
#include "urcu.hpp"

RCUSentinel RCU::urcu_register(int id) {
    return RCUSentinel(id, this);
}