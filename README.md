# userspaceRCUcpp

## What is it

A userspace rcu library for c++ with an easy to use interface, implemented in pure C++.

## What is provided

A urcu_read_lock primitive and the appropriate urcu_synchronize call, all wrapped nicely in a cozy C++ interface.

---
## Requirements
 C++11 or newer. Makefile assumes gcc compiler.

## How to include in my projects

1. Clone repository
2. Run make to produce the object file (change compilation flags)
3. Copy include/urcu.hpp and obj/urcu.o in your project directories
4. Link urcu.o when compiling and use header for declarations

## How to use

### In a nutshell

#### The RCU object

The RCU object is the first object which must be initialized to force rcu synchronization.
It is initialized with the number of threads it will observe. Each thread must call it's method,
`urcu_register_thread(int thread_id)` with a unique thread id from 0 to the number of threads declared.
A non unique id will lead to undefined behavior and is assumed as a user contract to avoid including a
complex indexing scheme. All threads registered to the same rcu object can be synchronized through a read 
lock.

The register method will return an RCUSentinel object with the unique thread id.

```c++
// simple
 RCU rcu(NUM_OF_RCU_THREADS);

```

### The RCUSentinel object

The RCUSentinel object has two purposes:

1. Call `urcu_read_lock()` to generate RCULock objects which will mark read critical areas.
2. Run `urcu_synchronize()` to wait for reader threads to finish before continuing.

```c++

// read thread code
void someThreadReadjob(RCU& rcu) {
    // the sentinel contains the unique thread id and will allow
    // synchronization with all the threads registered to the rcu
    // object which generated it
    auto sentinel = rcu.urcu_register_thread(my_thread_id);

    // lock is released when locked goes out of scope
    {
        auto locked = sentinel.urcu_read_lock();
        read_critical_area_stuff();
    }
}

void someThreadWriteJob(RCU& rcu) {
    auto sentinel = rcu.urcu_register_thread(my_thread_id);

    // wait for reader threads which started 
    // reading before the call to finish
    sentinel.urcu_synchronize();

    writeJob();
}

```

### The RCULock object details
The RCULock object read locks an area of code (as long as it is in scope) so that any `urcu_synchronize()` calls after it is created will wait until it unlocks. Unlocking happens through the destructor when the object goes out of scope.

Important details:

 1. Locks should not be nested or they will lead to undefined behavior
 2. Locks should only be created through the correspondent sentinel object

Complete example in examples.
