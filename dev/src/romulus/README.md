# Romulus #

Romulus is a Persistence Transactional Memory (PTM) which provides full ACID transactions [5]. 
It uses two instances of the data.
Transactions are "durable linearizable" [4].
There are four variants: "Romulus", "RomulusLog", "RomulusLR", "RomulusNI".



# How to build and run #

Edit the Makefile and change the -DPWB_IS_xxx setting depending on your supported architecture.
Possible options for PWB are:

    -DPWB_IS_STT          Emulates STT RAM with delays
    -DPWB_IS_PCM          Emulates PCM RAM with delays
    -DPWB_IS_CLFLUSH      pwb is a CLFLUSH and pfence/psync are nops      (Broadwell)
    -DPWB_IS_CLFLUSHOPT   pwb is a CLFLUSHOPT and pfence/psync are SFENCE (Kaby Lake) 
    -DPWB_IS_CLWB         pwb is a CLWB and pfence/psync are SFENCE       (SkyLake SP, or Ice Lake and beyond)
    -DPWB_IS_NOP          pwb/pfence/psync are nops. Used for shared memory persistence

Then type 'make' to build the benchmarks

    make

And go into the bin/ folder to check which executables were built.
If you want to see the benchmarks that compare with PMDK (formerly called NVML) you must install PMDK first, from https://github.com/pmem/pmdk
If you stop a benchmark halfway, make sure to clean the persistent memory afterwards with

    make persistencyclean


# For those of you in a rush #

If you just want to use romulus in your own stuff, then here is how.
Copy the following folders and files into your project and build everything together:

    romuluslog/RomulusLog.hpp
    romuluslog/RomulusLog.cpp
    romuluslog/malloc.cpp
    common/pfences.h
    common/ThreadRegistry.cpp
    common/ThreadRegistry.hpp
    common/CRWWPSpinLock.hpp

To build, you need to pass a flag to specify which instruction you want for PWB.
This depends on your machine. Take a look at common/pfences.h for options. If you have an Ice Lake or beyond add this compilation flag:
    
    -DPWB_IS_CLWB

Then in your code, include use with:
    
    #include "romuluslog/RomulusLog.hpp"
    using namespace romuluslog;
    


## Algorithms ##

### Romulus (pure) ###

The vanilla Romulus implementation (the slowest)
Files:

    romulus/Romulus.hpp              The class containing Romulus's Persistency Engine
    romulus/malloc.cpp               Doug Lea's memory allocator, annotated to be used with Romulus 
    rwlocks/CRWWPSpinLock.hpp        Our implementation of C-RW-WP [1]
    common/ThreadRegistry.hpp        A thread registration class to allow a dynamic number of threads 
    common/pfences.h                 Defines the persistency fence instructions to use as pwb/pfence/psync


### Romulus Log ###

The most generic implementation of Romulus. Use this implementation for most use cases. 
This implementation of Romulus has the log optimization. 
Calls to pwbs are deferred to commit time.
A global reader-writer lock (C-RW-WP) protects concurrent access, with writers being starvation-free. Uses flat combining.
Files:

    romuluslog/RomulusLog.hpp        The class containing RomulusLog's PTM
    romuluslog/malloc.c              Doug Lea's memory allocator, annotated to be used with RomulusLog
    rwlocks/CRWWPSpinLock.hpp        Our implementation of C-RW-WP [1]
    common/ThreadRegistry.hpp        A thread registration class to allow a dynamic number of threads 
    common/pfences.h                 Defines the persistency fence instructions to use as pwb/pfence/psync


### Romulus Left-Right ###

This implementation of Romulus has the log optimization. 
Calls to pwbs are deferred to commit time.
Uses the Left-Right concurrency mechanism [2] with our own URCU implementation [3], providing (blocking) starvation-free progress for writers and wait-free population oblivious progress for readers.
Files:

    romuluslr/RomulusLR.hpp       The class containing RomulusLR's PTM
    romuluslr/malloc.c            Doug Lea's memory allocator, annotated to be used with RomulusLR
    common/ThreadRegistry.hpp     A thread registration class to allow a dynamic number of threads 
    common/pfences.h              Defines the persistency fence instructions to use as pwb/pfence/psync


### Romulus Non-Interposing ###

This is a variant of the vanilla Romulus, but without any interposition.
A global reader-writer lock (C-RW-WP) protects concurrent access, with writers being starvation-free. Uses flat combining.
Files:

    romulusni/RomulusNI.hpp          The class containing RomulusLR's PTM
    romulusni/malloc.c               Doug Lea's memory allocator, annotated to be used with RomulusNI
    rwlocks/CRWWPSpinLock.hpp        Our implementation of C-RW-WP [1]
    common/ThreadRegistry.hpp        A thread registration class to allow a dynamic number of threads 
    common/pfences.h                 Defines the persistency fence instructions to use as pwb/pfence/psync



## Transactions in Romulus ##

Transactions in Romulus have the following properties:
- They provide flat nesting. Moreover it is allowed to nest read_transaction() inside a write_transaction(), but it is not possible to nest a write_transaction() in a read_transaction();
- Transactions are irrevocable;
- Transactions do not handle exceptions;
- User requested aborting is not supported. It is easy to add support for it, but it requires the disabling of the flat-combining technique so we don't want to do it;




## API ##

### Non-concurrent API ###

Non-concurrent transactions in persistency can be executed in two different ways. One way is with begin/end_transaction():
  using namespace romulus;
  Romulus::begin_transaction();
  user_code();
  Romulus::end_transaction();
  
Another way is to use lambdas:
  Romulus::transaction( [&] () {
    user_code();
  });
    

If there are variables inside the transaction whose type T has been annotated with persist<T>, then any store to that variable will trigger the store (and or load) interposing.
You can only have on PTM instance for now, but we can add more if needed, although it will require a specialized constructor and saving the reference to the specific PTM instance inside each persist<T> object.
Operator overloading in C++ does the interposing of the stores (and loads if needed) so as to add to the log or put the PWB fence as needed.
Allocation and de-allocation of objects in persistency must be done with pe.alloc() and pe.free(). All the members of the allocated type are expected to be annotated with persist<>, otherwise this doesn't make sense... don't forget, _all_ the members must be annotated!

For example:
using namespace romulus;

    struct Node {
        persist<Key*>    key;
        persist<Value*>  val;
        persist<int64_t> hash;
        persist<Node*>   next;
    }; // Notice that Node isn't persist<>, only its members
  
    void insertNode() {
        Romulus::begin_transaction();
        Node* newNode = Romulus::alloc<Node>(); // Allocate a node in persistent memory
        tail.next = newNode;  // implies store to persistency, through persist::pstore()
        tail = tail.next;     // implies store to persistency, through persist::pstore()
        Romulus::end_transaction();
    }
  
    void removeNode() {
        Romulus::begin_transaction();
        Node* node = head;
        head = node->next;   // implies store to persistency, through persist::pstore()
        Romulus::free(node);       // De-allocate the node allocated in persistent memory
        Romulus::end_transaction();
    }     


Objects marked with persist<> annotations which have been heap allocated (with new/malloc) will not trigger the PTM. This is important for uses cases where you want to do a lookup on a persistent map but you don't want to create the lookup key in persistence and you don't want to make a customized type for the key which doesn't have the persist<> annotations.


### Concurrent API ###

TODO: talk about write_transaction() and read_transaction()


  
  
### Comparing with PMDK ###
PMDK was formerly known as pmem.io at http://pmem.io/pmdk
Download and install with:

    git clone https://github.com/pmem/pmdk.git
    cd pmdk
    make
    sudo make install

To enable persistency fences and flushes on DRAM we need to set some environment variables when running the benchmarks
http://pmem.io/pmdk/manpages/linux/master/libpmem/libpmem.7.html

First, force pmdk to recognize DRAM as being persistent:

    export PMEM_IS_PMEM_FORCE=1

then, if you don't want the highest possible flush supported by your CPU, use the following

    export PMEM_NO_CLFLUSHOPT=1
    export PMEM_NO_CLWB=1
    
    make bench-pmdk    
  
  
### Detecting Memory leaks and errors in persistent memory ###
There is a specialized version of valgrind for persistent memory. You can install and use it with:

    git clone --recursive https://github.com/pmem/valgrind.git
    cd valgrind 
    ./autogen.sh
    ./configure
    make
    sudo make install

    valgrind --tool=pmemcheck bin/bench-sets-romlog


## More info ##

Talks that might be interesting
https://www.youtube.com/watch?v=-YtmGEvSweg
https://www.youtube.com/watch?v=mn42HgAjDug
https://www.youtube.com/watch?v=S3Fx-7avfs4


## Bibliography ##
[1] NUMA-aware reader-writer locks
https://dl.acm.org/citation.cfm?id=2442532

[2] Left-Right
https://github.com/pramalhe/ConcurrencyFreaks/blob/master/papers/left-right-2014.pdf

[3] Grace-sharing Userspace RCUs
https://github.com/pramalhe/ConcurrencyFreaks/blob/master/papers/gracesharingurcu-2017.pdf

[4] Linearizability of persistent memory objects under a full-system-crash failure model
https://www.cs.rochester.edu/u/scott/papers/2016_DISC_persistence.pdf

[5] Romulus: Efficient Algorithms for Persistent Transactional Memory
To appear in SPAA 2018
http://...

[6] Flat Combining and the Synchronization-Parallelism Tradeoff
https://people.csail.mit.edu/shanir/publications/Flat%20Combining%20SPAA%2010.pdf



