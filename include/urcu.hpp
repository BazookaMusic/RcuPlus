// Copyright 2019 Sotiris Dragonas
#ifndef USERSPACERCU_INCLUDE_URCU_HPP_
    #define USERSPACERCU_INCLUDE_URCU_HPP_

    // used for line sharing
    #ifndef URCU_CACHE_SIZE
        #define URCU_CACHE_LINE 64
    #endif

    #include <cassert>
    #include <atomic>
    #include <mutex>
    #include <iostream>
    #include <memory>

namespace URCU {
    static_assert(URCU_CACHE_LINE > sizeof(std::atomic<int64_t>), "too small cache line size given");


    class RCUSentinel;
 
    class RCU {
        friend class RCUSentinel;
        friend class RCULock;
     private:
            struct  RCUNode {
                std::atomic<int64_t> time;
                char pad_to_align[URCU_CACHE_LINE - sizeof(std::atomic<int64_t>)];
            };

            int threads;
            RCUNode** rcu_table;
            std::atomic<int> curr_thread_index;
            int* free_indices_stack;
            int stack_index;
            std::mutex stack_lock;

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
            RCUSentinel urcu_register_thread();

    };


    class RCULock {
     private:
            const int index;
            RCU::RCUNode** _rcu_table;
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
        RCULock(const int i, RCU::RCUNode** rcu_table, const int threads);
        ~RCULock(void);
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

            // releases slot for new threads to register
            ~RCUSentinel();


            // urcu_read_lock: Create an RCULock object for
            // the registered thread
            RCULock urcu_read_lock() {
                return RCULock(index, rcu->rcu_table, rcu->threads);
            }
            
            // unregister thread
            void urcu_unregister_thread();

            // wait for previously created read locks
            void urcu_synchronize();

            // returns the unique internal id
            // of the thread
            // On deregister older ids may become available
            // for reuse by new threads
            int get_rcu_thread_id() const {
                return index;
            }

    };
}
#endif  // USERSPACERCU_INCLUDE_URCU_HPP_


