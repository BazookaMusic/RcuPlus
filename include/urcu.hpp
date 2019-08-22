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
        RCULock(const int i, RCUNode** rcu_table, const int threads);
        ~RCULock(void);
    };





    class RCU {
        friend class RCUSentinel;

     private:
            int threads;
            RCUNode** rcu_table;

     public:
            explicit RCU(int num_threads);

            ~RCU();

            RCUSentinel urcu_register_thread(int thread_id);
    };

    class RCUSentinel {
     private:
            const int index;
            const RCU* rcu;
            int64_t *times;

     public:
            RCUSentinel(const int id, const RCU* _rcu);

            ~RCUSentinel();



            // RCULock object creates a read lock,
            // which is released when it goes out of scope
            // or it's destructor is called
            inline RCULock urcu_read_lock() {
                return RCULock(index, rcu->rcu_table, rcu->threads);
            }

            // wait for previously created read locks
            void urcu_synchronize();
    };
#endif  // USERSPACERCU_INCLUDE_URCU_HPP_


