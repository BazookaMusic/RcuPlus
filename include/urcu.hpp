// Copyright 2019 Sotiris Dragonas
#ifndef USERSPACERCU_INCLUDE_URCU_HPP_
    #define USERSPACERCU_INCLUDE_URCU_HPP_

    #include <assert.h>
    #include <atomic>
    #include <iostream>


    typedef struct RCUNode {
        std::atomic<int64_t> time;
    } RCUNode;

    class RCUSentinel;
    class RCU;


    class RCULock {
     private:
            const int index;
            RCUNode** _rcu_table;
            const int threads;

     public:
        // RCULock: Read locks its scope after its creation,
        // unlocks when out of scope
        RCULock(const int i, RCUNode** rcu_table, const int threads);
        ~RCULock(void);
    };





    class RCU {
        friend class RCUSentinel;

     private:
            int threads;
            RCUNode** rcu_table;

     public:
            // RCU: Allows registering threads for read locking
            // and synchronizing writes
            explicit RCU(int num_threads);

            ~RCU();
            // urcu_register_thread: register a thread to the rcu service
            // and return an object to create read locks and synchronize
            // with readers. thread_id should be unique and between 0 and
            // number of threads
            RCUSentinel urcu_register_thread(int thread_id);
    };

    class RCUSentinel {
     private:
            const int index;
            const RCU* rcu;
            int64_t *times;

     public:
            // RCUSentinel: allows a registered thread to
            // create RCULocks and to wait for previously
            // created locks. Requires a unique id for each thread
            // from 0 to number of threads - 1. Non-unique ids
            // will cause undefined behavior.
            RCUSentinel(const int id, const RCU* _rcu);

            ~RCUSentinel();


            // urcu_read_lock: Create an RCULock object for
            // the registered thread
            inline RCULock urcu_read_lock() {
                return RCULock(index, rcu->rcu_table, rcu->threads);
            }

            // wait for previously created read locks
            void urcu_synchronize();
    };
#endif  // USERSPACERCU_INCLUDE_URCU_HPP_


