// Copyright 2019 Sotiris Dragonas
#ifndef USERSPACERCU_INCLUDE_URCU_HPP_
    #define USERSPACERCU_INCLUDE_URCU_HPP_

    // used for line sharing
    #ifndef URCU_CACHE_SIZE
        #define URCU_CACHE_LINE 64
    #endif

    #include <cassert>
    #include <atomic>
    #include <iostream>
    #include <memory>


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
            bool valid;

     public:
        // delete copy constructor to avoid
        // double references
        RCULock(const RCULock&) = delete;
        
        RCULock(RCULock&& a_lock);

        RCULock& operator=(const RCULock&) = delete;

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
            RCU(const RCU&) = delete;
            RCU& operator=(const RCU&) = delete;

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
            RCU* rcu;
            int64_t *times;

     public:
            RCUSentinel& operator=(const RCUSentinel&) = delete;
            RCUSentinel(const RCUSentinel&) = delete;

            // only move constructor
            RCUSentinel(RCUSentinel&& a_sentinel);
           

            // RCUSentinel: allows a registered thread to
            // create RCULocks and to wait for previously
            // created locks. Requires a unique id for each thread
            // from 0 to number of threads - 1. Non-unique ids
            // will cause undefined behavior.
            RCUSentinel(const int id, RCU* _rcu);

            ~RCUSentinel();


            // urcu_read_lock: Create an RCULock object for
            // the registered thread
            RCULock urcu_read_lock() {
                return RCULock(index, rcu->rcu_table, rcu->threads);
            }

            // wait for previously created read locks
            void urcu_synchronize();
    };
#endif  // USERSPACERCU_INCLUDE_URCU_HPP_


