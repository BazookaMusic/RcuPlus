// Copyright 2019 Sotiris Dragonas

#include <chrono>
#include <sstream>
#include <iostream>
#include <ctime>
#include <thread>

#include "include/catch.hpp"
#include "include/urcu.hpp"


const int RCU_THREADS = 100;

// simulates a read critical area
void read_critical_area(int thread_index, std::atomic<int> &counter, RCU& rcu) {
    auto sentinel = rcu.urcu_register_thread(thread_index);

    // lock read area
    RCULock locked = sentinel.urcu_read_lock();

    // read
    counter++;

    // add small delay to make sure that
    // wait thread starts before end
    // fuzzy test, should only fish out weird issues
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// simulates a waiting thread
void wait_for_readers(int thread_index, std::atomic<int> &counter, 
                        RCU& rcu, bool &success) {
    auto sentinel = rcu.urcu_register_thread(thread_index);

    sentinel.urcu_synchronize();

    // All threads should be finished
    if (counter == RCU_THREADS - 1) {
        success = true;
    }
}


// should fail if wait thread does not properly wait for read threads
int RCUTest() {
    std::cout << "Testing RCU Read Locks and Synchronize" << std::endl;
    RCU rcu(RCU_THREADS);

    std::thread threads[RCU_THREADS];

    std::atomic<int> readCounter;

    readCounter = 0;

    bool success = false;



    for (int i = 0; i < RCU_THREADS - 1; i++) {
        threads[i] =
            std::thread(
                read_critical_area, i,
                std::ref(readCounter), std::ref(rcu));
    }

    threads[RCU_THREADS-1] =
    std::thread(wait_for_readers,
        RCU_THREADS-1,
        std::ref(readCounter),
        std::ref(rcu),
        std::ref(success));


    for (int i = 0; i < RCU_THREADS; i++) {
         threads[i].join();
    }

    if (success) {
        std::cout<< "ALL THREADS PROPERLY SYNCED" << std::endl;
    } else {
        std::cout<< "WRITE HAPPENED BEFORE READ" << std::endl;
    }

    return success;
}


TEST_CASE("RCU Test", "[rcu]") {
    std::cout << "RCU TEST" <<std::endl;
    REQUIRE(RCUTest());
}
