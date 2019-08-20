// Copyright 2019 Sotiris Dragonas
#ifndef URCU_HPP_
   
    #define URCU_HPP

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
            RCULock(const int i, RCUNode** rcu_table, const int threads):
             index(i), _rcu_table(rcu_table), threads(threads) {
                assert(threads > 0 && index < threads && _rcu_table[index]);
                _rcu_table[index]->time += 1;
            }

            ~RCULock() {
                assert(threads > 0 && index < threads && _rcu_table[index]);
                // set lsb
                _rcu_table[index]->time |= 1;
            }
    };





    class RCU {
        
        friend class RCUSentinel;

     private:
            int threads;
            RCUNode** rcu_table;

     public:
            explicit RCU(int num_threads): threads(num_threads) {
                rcu_table = new RCUNode*[threads];

                RCUNode* new_node;
                for (int i = 0; i < threads; i++) {
                    new_node = new RCUNode;
                    new_node->time = 1;
                    rcu_table[i] = new_node;
                }
            }

            ~RCU() {
                for (int i = 0; i < threads; i++) {
                    delete rcu_table[i];
                }

                delete [] rcu_table;
            }

            RCUSentinel urcu_register(int id);

    };

    class RCUSentinel {
     private:
            const int index;
            const RCU* rcu;
            int64_t *times;

     public:
            RCUSentinel(const int id, const RCU* _rcu): index(id), rcu(_rcu) {
                assert(rcu);
                times = new int64_t[rcu->threads];
            }

            ~RCUSentinel() {
                delete [] times;
            }



            // RCULock object creates a read lock,
            // which is released when it goes out of scope
            // or it's destructor is called
            RCULock urcu_read_lock() {
                return RCULock(index, rcu->rcu_table, rcu->threads);
            }


            void urcu_synchronize() {
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
    };
#endif


