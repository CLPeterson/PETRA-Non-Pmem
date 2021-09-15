#ifndef _PMDK_TM_PERSISTENCY_
#define _PMDK_TM_PERSISTENCY_


#ifdef PMDK_PTM
#include <shared_mutex>         // You can comment this out if you use instead our C-RW-WP reader-writer lock
#include <libpmemobj++/p.hpp>
#include <libpmemobj++/transaction.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/allocator.hpp>
#endif

namespace pmdk {

#ifdef PMDK_PTM

using namespace pmem::obj;

auto gpop = pool_base::create("/dev/shm/pmdk_shared", "", (size_t)(400*1024*1024));

std::shared_timed_mutex grwlock {};

#endif


/*
 * <h1> Wrapper for libpmemobj from pmem.io </h1>
 *
 * http://pmem.io/pmdk/cpp_obj/
 *
 */
class PMDKTM {

public:
    PMDKTM()  { }

    ~PMDKTM() { }


    static std::string className() { return "PMDK"; }


    template <typename T>
    static inline T* get_object(int idx) {
        return nullptr;
        //return (T*)per->objects[idx];  // TODO: fix me
    }

    template <typename T>
    static inline void put_object(int idx, T* obj) {
        //per->objects[idx] = obj;  // TODO: fix me
        //PWB(&per->objects[idx]);
    }


    inline void begin_transaction() {
    }

    inline void end_transaction() {
    }

    inline void recover_if_needed() {
    }

    inline void abort_transaction(void) {
    }


    static void init() {
        // TODO: accept params here
    }

    template<class F>
    static void transaction(F&& func) {
#ifdef PMDK_PTM
        transaction::exec_tx(gpop, func);
#endif
    }

    template<class F>
    static void write_transaction(F&& func) {
#ifdef PMDK_PTM
        grwlock.lock();
        transaction::exec_tx(gpop, func);
        grwlock.unlock();
#endif
    }

    template<class F>
    static void read_transaction(F&& func) {
#ifdef PMDK_PTM
        grwlock.lock_shared();
        transaction::exec_tx(gpop, func);
        grwlock.unlock_shared();
#endif
    }


    /*
     * Allocator
     * Must be called from within a transaction
     */
    template <typename T, typename... Args>
    static T* alloc(Args&&... args) {
        void *addr = nullptr;
#ifdef PMDK_PTM
        auto oid = pmemobj_tx_alloc(sizeof(T), 0);
        addr = pmemobj_direct(oid);
#endif
        return new (addr) T(std::forward<Args>(args)...); // placement new
    }


    /*
     * De-allocator
     * Must be called from within a transaction
     */
    template<typename T>
    static void free(T* obj) {
#ifdef PMDK_PTM
        if (obj == nullptr) return;
        obj->~T();
        pmemobj_tx_free(pmemobj_oid(obj));
#endif
    }

    /* Allocator for C methods */
    static void* pmalloc(size_t size) {
        void* ptr = nullptr;
#ifdef PMDK_PTM
        auto oid = pmemobj_tx_alloc(size, 0);
        ptr = pmemobj_direct(oid);
#endif
        return ptr;
    }


    /* De-allocator for C methods (like memcached) */
    static void pfree(void* ptr) {
#ifdef PMDK_PTM
        pmemobj_tx_free(pmemobj_oid(ptr));
#endif
    }

    // Doesn't actually do any checking. That functionality exists only for RomulusLog and RomulusLR
    static bool consistency_check(void) {
        return true;
    }
};


/*
 * Definition of persist<> type
 */
template<typename T>
struct persist {
#ifdef PMDK_PTM
    // Stores the actual value
    pmem::obj::p<T> val;   // This is where the magic happens in libpmemobj
#else
    T val;
#endif
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
#ifdef PMDK_PTM
        return (T*)&val.get_ro();  // tsc, tsc: bad way to take away constness, but p<> is inflexible
#else
        return &val;
#endif
    }

    // Copy constructor
    persist<T>(const persist<T>& other) {
        pstore(other.pload());
    }

    // Assignment operator
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
    }

    inline T pload() const {
        return val;
    }
};




} // end of pmdk namespace

#endif   // _PMDK_TM_PERSISTENCY_
