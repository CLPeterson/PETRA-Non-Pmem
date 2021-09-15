/*
 * Copyright 2017-2018
 *   Andreia Correia <andreia.veiga@unine.ch>
 *   Pascal Felber <pascal.felber@unine.ch>
 *   Pedro Ramalhete <pramalhe@gmail.com>
 *
 * This work is published under the MIT license. See LICENSE.TXT
 */
#ifndef _ROMULUS_PERSISTENCY_H_
#define _ROMULUS_PERSISTENCY_H_

#include <atomic>
#include <cstdint>
#include <cassert>
#include <string>
#include <cstring>      // std::memcpy()
#include <sys/mman.h>   // Needed if we use mmap()
#include <sys/types.h>  // Needed by open() and close()
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>     // Needed by close()
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <functional>
#include <set>          // Needed by allocation statistics

#include "../commonromulus/pfences.h"
#include "../commonromulus/ThreadRegistry.hpp"
#include "../rwlocks/CRWWPSpinLock.hpp"    // Not needed for single-threaded applications

namespace romulus{
/*
 * <h1> Romulus </h1>
 * TODO: explain this...
 *
 *
 *
 */

// Forward declaration of Romulus to create a global instance
class Romulus;
extern Romulus gRom;

// Global with the 'main' size. Used by pload()
extern uint64_t g_main_size;
// Global with the 'main' addr. Used by pload()
extern uint8_t* g_main_addr;

// Counter of nested write transactions
extern thread_local int64_t tl_nested_write_trans;
// Counter of nested read-only transactions
extern thread_local int64_t tl_nested_read_trans;


typedef void* mspace;
extern void* mspace_malloc(mspace msp, size_t bytes);
extern void mspace_free(mspace msp, void* mem);
extern mspace create_mspace_with_base(void* base, size_t capacity, int locked);

/*
 * Definition of persist<> type
 * In Romulus we interpose the stores only to do a PWB()
 */
template<typename T>
struct persist {
    // Stores the actual value
    T val {};

    persist() { }

    persist(T initVal) {
        pstore(initVal);
    }

    // Casting operator
    operator T() {
        return pload();
    }

    // Prefix increment operator: ++x
    void operator++ () {
        pstore(pload()+1);
    }

    // Prefix decrement operator: --x
    void operator-- () {
        pstore(pload()-1);
    }

    void operator++ (int) {
        pstore(pload()+1);
    }

    void operator-- (int) {
        pstore(pload()-1);
    }

    // Equals operator: first downcast to T and then compare
    bool operator == (const T& otherval) const {
        return pload() == otherval;
    }

    // Difference operator: first downcast to T and then compare
    bool operator != (const T& otherval) const {
        return pload() != otherval;
    }

    // Relational operators
    bool operator < (const T& rhs) {
        return pload() < rhs;
    }
    bool operator > (const T& rhs) {
        return pload() > rhs;
    }
    bool operator <= (const T& rhs) {
        return pload() <= rhs;
    }
    bool operator >= (const T& rhs) {
        return pload() >= rhs;
    }

    T operator % (const T& rhs) {
        return pload() % rhs;
    }

    // Operator arrow ->
    T operator->() {
        return pload();
    }

    // Operator &
    T* operator&() {
        return &val;
    }

    // Copy constructor
    persist<T>(const persist<T>& other) {
        pstore(other.pload());
    }

    // Assignment operator from an atomic_mwc
    persist<T>& operator=(const persist<T>& other) {
        pstore(other.pload());
        return *this;
    }

    // Assignment operator from a value
    persist<T>& operator=(T value) {
        pstore(value);
        return *this;
    }

    persist<T>& operator&=(T value) {
        pstore(pload() & value);
        return *this;
    }

    persist<T>& operator|=(T value) {
        pstore(pload() | value);
        return *this;
    }
    persist<T>& operator+=(T value) {
        pstore(pload() + value);
        return *this;
    }
    persist<T>& operator-=(T value) {
        pstore(pload() - value);
        return *this;
    }

    inline void pstore(T newVal) {
        val = newVal;
        const uint8_t* valaddr = (uint8_t*)&val;
        if (valaddr > g_main_addr && valaddr < g_main_addr+g_main_size) PWB(&val);
    }

    inline T pload() const {
        return val;
    }
};




class Romulus {
    // Id for sanity check of Romulus
    static const uint64_t MAGIC_ID = 0x1337BAB1;

    // Number of indexes available for put_object/get_object
    static const int NUM_ROOT_PTRS = 100;

    // Possible values for "state"
    static const int IDLE = 0;
    static const int MUTATING = 1;
    static const int COPYING = 2;

    // Size in bytes of a chunk in the log
    static const int CHUNK_SIZE = 1024;

    // Filename for the mapping file
    const char* MMAP_FILENAME = "/dev/shm/romulus_shared";

    // Member variables
    bool dommap;
    int fd = -1;
    uint8_t* base_addr;
    uint64_t max_size;
    uint8_t* back_addr;
    CRWWPSpinLock rwlock {};
    // Stuff use by the Flat Combining mechanism
    static const int CLPAD = 128/sizeof(uintptr_t);
    alignas(128) std::atomic< std::function<void()>* >* fc; // array of atomic pointers to functions
    const int maxThreads;

    // One instance of this is at the start of base_addr, in persistent memory
    struct PersistentHeader {
        uint64_t           id {0}; //validation
        std::atomic<int>   state {IDLE};
        persist<void*>*    objects {nullptr};   // directory
        mspace             ms {};
        uint64_t used_size = 0;//it has to be the last, to calculate the used_size
    };

    PersistentHeader* per {nullptr};

    // Set to true to get a report of leaks on the destructor
    bool enableAllocStatistics = false;

    struct AStats {
        void*    addr;
        uint64_t size;
        AStats(void* addr, uint64_t size) : addr{addr}, size{size} {}
        bool operator < (const AStats& rhs) const {
            return addr < rhs.addr;
        }
    };

    std::set<AStats> statsSet {};
    uint64_t statsAllocBytes {0};
    uint64_t statsAllocNum {0};

    //
    // Private methods
    //

    // Flush touched cache lines
    inline void flush_range(uint8_t* addr, size_t length) {
        const int cache_line_size = 64;
        uint8_t* ptr = addr;
        uint8_t* last = addr + length;
        for (; ptr < last; ptr += cache_line_size) PWB(ptr);
    }

    //
    // Private methods
    //

    void copyMainToBack() {
        // Copy the data from 'main' to 'back'
        uint64_t size = std::min(per->used_size, g_main_size);
        std::memcpy(back_addr, g_main_addr, size);
        flush_range(back_addr, size);}

    void copyBackToMain() {
        // Copy the data from 'back' to 'main'
        uint64_t size = std::min(per->used_size, g_main_size);
        std::memcpy(g_main_addr, back_addr, size);
        flush_range(g_main_addr, size);
    }

    void compareMainAndBack() {
        if(std::memcmp(g_main_addr, back_addr, g_main_size)!=0){
            int sumdiff = 0;
            for (size_t idx = 0; idx < g_main_size; idx++) {
                if (*(g_main_addr+idx) != *(back_addr+idx)) {
                    std::cout<<"main_addr "<<g_main_addr<<" idx "<<idx<<" back_addr "<<back_addr<<"\n";
                    printf("Difference at %p  main=%ld  back=%ld\n",g_main_addr+idx, *(int64_t*)(g_main_addr+idx), *(int64_t*)(back_addr+idx));
                    sumdiff++;
                }
            }
            if (sumdiff != 0) printf("sumdiff=%d bytes\n", sumdiff);
            assert(sumdiff == 0);
        }
    }

    //void copyMainToBack();
    //void copyBackToMain();


public:

    Romulus() : dommap{true},maxThreads{128}{

        fc = new std::atomic< std::function<void()>* >[maxThreads*CLPAD];
        for (int i = 0; i < maxThreads; i++) {
            fc[i*CLPAD].store(nullptr, std::memory_order_relaxed);
        }
        ns_init();
/*
        if (dommap) {

        }*/
    }


    ~Romulus() {
        delete[] fc;
        // Must do munmap() if we did mmap()
        if (dommap) {
            //destroy_mspace(ms);
            munmap(base_addr, max_size);
            close(fd);
        }
        if (enableAllocStatistics) { // Show leak report
            std::cout << "Romulus: statsAllocBytes = " << statsAllocBytes << "\n";
            std::cout << "Romulus: statsAllocNum = " << statsAllocNum << "\n";
        }
    }

    void ns_init(){
        base_addr = (uint8_t*)0x7fdd00000000;
        max_size = 400*1024*1024; // 400 Mb => 200 Mb for the user
        // Check if the file already exists or not
        struct stat buf;
        if (stat(MMAP_FILENAME, &buf) == 0) {
            // File exists
            //std::cout << "Re-using memory region\n";
            fd = open(MMAP_FILENAME, O_RDWR|O_CREAT, 0755);
            assert(fd >= 0);
            // mmap() memory range
            uint8_t* got_addr = (uint8_t *)mmap(base_addr, max_size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
            if (got_addr == MAP_FAILED) {
                printf("got_addr = %p  %p\n", got_addr, MAP_FAILED);
                perror("ERROR: mmap() is not working !!! ");
                assert(false);
            }
            per = reinterpret_cast<PersistentHeader*>(base_addr);
            if(per->id != MAGIC_ID) createFile();
            g_main_size = (max_size - sizeof(PersistentHeader))/2;
            g_main_addr = base_addr + sizeof(PersistentHeader);
            back_addr = g_main_addr + g_main_size;
            recover();
        } else {
            createFile();
        }
    }

    void createFile(){
        // File doesn't exist
        fd = open(MMAP_FILENAME, O_RDWR|O_CREAT, 0755);
        assert(fd >= 0);
        if (lseek(fd, max_size-1, SEEK_SET) == -1) {
            perror("lseek() error");
        }
        if (write(fd, "", 1) == -1) {
            perror("write() error");
        }
        // mmap() memory range
        uint8_t* got_addr = (uint8_t *)mmap(base_addr, max_size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
        if (got_addr == MAP_FAILED) {
            printf("got_addr = %p  %p\n", got_addr, MAP_FAILED);
            perror("ERROR: mmap() is not working !!! ");
            assert(false);
        }
        // No data in persistent memory, initialize
        per = new (base_addr) PersistentHeader;
        g_main_size = (max_size - sizeof(PersistentHeader))/2;
        g_main_addr = base_addr + sizeof(PersistentHeader);
        back_addr = g_main_addr + g_main_size;
        PWB(&per->state);

        begin_transaction();
        per->ms = create_mspace_with_base(g_main_addr, g_main_size, false);
        per->objects = (persist<void*>*)mspace_malloc(per->ms, sizeof(void*)*NUM_ROOT_PTRS);
        for (int i = 0; i < NUM_ROOT_PTRS; i++) {
            per->objects[i] = nullptr;
        }
        per->used_size = g_main_size;
        end_transaction();

        per->used_size = (uint8_t*)(&per->used_size) - ((uint8_t*)base_addr+sizeof(PersistentHeader))+128;
        flush_range((uint8_t*)per,sizeof(PersistentHeader));
        PFENCE();
        per->id = MAGIC_ID;
        PWB(&per->id);
        PSYNC();
    }

    static std::string className() { return "Romulus"; }

    template <typename T>
    static inline T* get_object(int idx) {
        return (T*)gRom.per->objects[idx].pload();
    }

    template <typename T>
    static inline void put_object(int idx, T* obj) {
        gRom.per->objects[idx] = obj;
    }

    /*
     * Must be called at the beginning of each (write) transaction.
     */
    inline void begin_transaction() {
        // Check for nested write transaction
        ++tl_nested_write_trans;
        if (tl_nested_write_trans != 1) return;
        per->state.store(MUTATING, std::memory_order_relaxed);
        PWB(&per->state);
        // One PFENCE() is enough for all user modifications because no ordering is needed between them.
        PFENCE();
    }


    /*
     * Must be called at the end of each (write) transaction.
     */
    inline void end_transaction() {
        // Check for nested transaction
        --tl_nested_write_trans;
        if (tl_nested_write_trans != 0) return;
        // Do a PFENCE() to make persistent the stores done in 'main' and on
        // the Romulus persistent data (due to memory allocation). We only care
        // about ordering here, not about durability, therefore, no need to block.
        PFENCE();
        per->state.store(COPYING, std::memory_order_relaxed);
        PWB(&per->state);
        PWB(&per->used_size);
        // PSYNC() here to have ACID Durability on the mutations done to 'main' and make the change of state visible
        PSYNC();
        copyMainToBack();
        PFENCE();
        per->state.store(IDLE, std::memory_order_relaxed);
    }



    /*
     * Recovers from an incomplete transaction if needed
     */
    inline void recover() {
        int lstate = per->state.load(std::memory_order_relaxed);
        if (lstate == IDLE) {
            return;
        } else if (lstate == COPYING) {
            printf("Romulus: Recovery from COPYING...\n");
            copyMainToBack();
        } else if (lstate == MUTATING) {
            printf("Romulus: Recovery from MUTATING...\n");
            copyBackToMain();
        } else {
            assert(false);
            // ERROR: corrupted state
        }
        PFENCE();
        per->state.store(IDLE, std::memory_order_relaxed);
        return;
    }


    // Same as begin/end transaction, but with a lambda.
    // Calling abort_transaction() from within the lambda is not allowed.
    template<typename R, class F>
    R transaction(F&& func) {
        begin_transaction();
        R retval = func();
        end_transaction();
        return retval;
    }

    template<class F>
    static void transaction(F&& func) {
        gRom.begin_transaction();
        func();
        gRom.end_transaction();
    }


    // Non-static thread-safe read-write transaction
    template<class F>
    void ns_write_transaction(F&& func) {
        if (tl_nested_write_trans > 0) {
            func();
            return;
        }
        std::function<void()> myfunc = func;
        int tid = ThreadRegistry::getTID();
        // Add our mutation to the array of flat combining
        fc[tid*CLPAD].store(&myfunc);
        // Lock writersMutex
        while (true) {
            if (rwlock.tryExclusiveLock()) break;
            // Check if another thread executed my mutation
            if (fc[tid*CLPAD].load(std::memory_order_acquire) == nullptr) return;
            std::this_thread::yield();
        }

        bool somethingToDo = false;
        const int maxTid = ThreadRegistry::getMaxThreads();

        rwlock.waitForReaders();
        // Save a local copy of the flat combining array
        std::function<void()>* lfc[maxTid];
        for (int i = 0; i < maxTid; i++) {
            lfc[i] = fc[i*CLPAD].load(std::memory_order_acquire);
            if (lfc[i] != nullptr) somethingToDo = true;
        }
        // Check if there is at least one operation to apply
        if (!somethingToDo) {
            rwlock.exclusiveUnlock();
            return;
        }

        ++tl_nested_write_trans;
        per->state.store(MUTATING, std::memory_order_relaxed);
        PWB(&per->state);
        // One PFENCE() is enough for all user modifications because no ordering is needed between them.
        PFENCE();
        // Apply all mutativeFunc
        for (int i = 0; i < maxTid; i++) {
            if (lfc[i] == nullptr) continue;
            (*lfc[i])();
        }
        PFENCE();
        per->state.store(COPYING, std::memory_order_relaxed);
        PWB(&per->state);
        // PSYNC() here to have ACID Durability on the mutations done to 'main' and make the change of state visible
        PSYNC();
        // After changing changing state to COPYING all applied mutativeFunc are visible and persisted
        for (int i = 0; i < maxTid; i++) {
            if (lfc[i] == nullptr) continue;
            fc[i*CLPAD].store(nullptr, std::memory_order_release);
        }

        copyMainToBack();

        PFENCE();
        per->state.store(IDLE, std::memory_order_relaxed);
        rwlock.exclusiveUnlock();
        --tl_nested_write_trans;
    }

    // Non-static Thread-safe read-only transaction
    template<class F>
    void ns_read_transaction(F&& func) {
        if (tl_nested_read_trans > 0) {
            func();
            return;
        }
        int tid = ThreadRegistry::getTID();
        ++tl_nested_read_trans;
        rwlock.sharedLock(tid);
        func();
        rwlock.sharedUnlock(tid);
        --tl_nested_read_trans;
    }

    /*
     * Mean to be called from user code when something bad happens and the whole
     * transaction needs to be aborted.
     * This function has strict semantics.
     */
    inline void abort_transaction(void) {
        // Check for nested transaction
        --tl_nested_write_trans;
        if (tl_nested_write_trans != 0) return;
        copyBackToMain();
        PFENCE();
        per->state.store(IDLE, std::memory_order_release);
        PWB(&per->state);
        // ACID Durability is needed when modifying "main", so do PSYNC
        PSYNC();
    }


    template <typename T, typename... Args>
    static T* alloc(Args&&... args) {
        Romulus& r = gRom;
        void* addr = mspace_malloc(r.per->ms, sizeof(T));
        assert(addr != 0);
        T* ptr = new (addr) T(std::forward<Args>(args)...); // placement new
        if (r.per->used_size < (uint8_t*)addr - g_main_addr + sizeof(T) + 128) {
            r.per->used_size = (uint8_t*)addr - g_main_addr + sizeof(T) + 128;
            PWB(&r.per->used_size);
        }
        if (r.enableAllocStatistics) {
            r.statsAllocBytes += sizeof(T);
            r.statsAllocNum++;
            r.statsSet.insert({addr, sizeof(T)});
        }
        return ptr;
    }


    /*
     * De-allocator
     * Calls destructor of T and then reclaims the memory using Doug Lea's free
     */
    template<typename T>
    static void free(T* obj) {
        if (obj == nullptr) return;
        obj->~T();
        Romulus& r = gRom;
        mspace_free(r.per->ms,obj);
        if (r.enableAllocStatistics) {
            auto search = r.statsSet.find({obj,0});
            if (search == r.statsSet.end()) {
                std::cout << "Attemped free() of unknown address\n";
                assert(false);
                return;
            }
            r.statsAllocBytes -= search->size;
            r.statsAllocNum--;
            r.statsSet.erase(*search);
        }
    }

    /* Allocator for C methods (like memcached) */
    static void* pmalloc(size_t size) {
        Romulus& r = gRom;
        void* addr = mspace_malloc(r.per->ms, size);
        assert (addr != 0);
        if (r.per->used_size < (uint8_t*)addr - g_main_addr + size + 128) {
            r.per->used_size = (uint8_t*)addr - g_main_addr + size + 128;
            PWB(&r.per->used_size);
        }
        if (r.enableAllocStatistics) {
            r.statsAllocBytes += size;
            r.statsAllocNum++;
            r.statsSet.insert({addr, size});
        }
        return addr;
    }

    /* De-allocator for C methods (like memcached) */
    static void pfree(void* ptr) {
        Romulus& r = gRom;
        mspace_free(r.per->ms, ptr);
        if (r.enableAllocStatistics) {
            auto search = r.statsSet.find({ptr,0});
            if (search == r.statsSet.end()) {
                std::cout << "Attemped pfree() of unknown address\n";
                assert(false);
                return;
            }
            r.statsAllocBytes -= search->size;
            r.statsAllocNum--;
            r.statsSet.erase(*search);
        }
    }


    static void init() {
        gRom.ns_init();
    }

    template<class F>
    static void read_transaction(F&& func) {
        gRom.ns_read_transaction(func);
    }

    template<class F>
    static void write_transaction(F&& func) {
        gRom.ns_write_transaction(func);
    }

    // Doesn't actually do any checking. That functionality exists only for RomulusLog and RomulusLR
    static bool consistency_check(void) {
        return true;
    }
};
}
#endif   // _ROMULUS_PERSISTENCY_H_
