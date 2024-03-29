# RcuPlus

[![CircleCI](https://circleci.com/gh/BazookaMusic/RcuPlus/tree/master.svg?style=svg)](https://circleci.com/gh/BazookaMusic/RcuPlus/tree/master)

## What is it

A lighweight userspace rcu library for c++ with an easy to use interface, implemented in pure C++, inspired by
the api detailed in [liburcu](https://liburcu.org/).

## What is provided

A urcu_read_lock primitive and the appropriate urcu_synchronize call, all wrapped nicely in a cozy C++ interface.
It allows writer threads to wait for previous reader threads to finish before continuing the execution.

## Requirements
 C++11 or newer. Makefile assumes gcc compiler, but clang also compiles properly.

## How to include in my projects

1. Clone repository
2. Run make to produce the object file (change compilation flags)
3. Copy include/urcu.hpp and obj/urcu.o in your project directories
4. Link urcu.o when compiling and use header for declarations

## How to use

#### The RCU object

The RCU object is the first object which must be initialized to force rcu synchronization.
It is initialized with the number of threads it will observe. Each thread must call it's method,
`urcu_register_thread()`.

The register method will return an RCUSentinel object.

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
    auto sentinel = rcu.urcu_register_thread();

    // lock is released when locked goes out of scope
    {
        auto locked = sentinel.urcu_read_lock();
        read_critical_area_stuff();
    }
}

void someThreadWriteJob(RCU& rcu) {
    auto sentinel = rcu.urcu_register_thread();

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

### Copying objects
All objects are only moveable as copying them would make no sense. Using a moved object
is undefined behavior.

### Cache line sharing
To eliminate cache line sharing by different threads, the parameter URCU_CACHE_LINE can be defined on compilation.
By default, it's set to 64 bytes.

## Examples
Run `make examples` to compile examples. The example executable will be in the examples folder.

## Tests
The project is painlessly tested courtesy of [Catch2](https://github.com/catchorg/Catch2).
Run `make run-tests` to run the tests. 
