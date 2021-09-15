#ifndef _MNEMOSYNE_PERSISTENCY_
#define _MNEMOSYNE_PERSISTENCY_

#include <pmalloc.h>
#include "pvar.h"

// We have to do this because pmalloc.h defines these as macros
#undef pmalloc
#undef pfree

namespace mnemosyne {


/*
 * <h1> Wrapper for Mnemosyne </h1>
 *
 * Mnemosyne was taken from here
 * https://github.com/snalli/mnemosyne-gcc
 *
 */
class Mnemosyne {
public:
    Mnemosyne()  { }

    ~Mnemosyne() { }


    static std::string className() { return "Mnemosyne"; }


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


    template<class F>
    void transaction(F&& func) {
        PTx { func(); }
    }

    template<class F>
    static void write_transaction(F&& func) {
        PTx { func(); }
    }

    template<class F>
    static void read_transaction(F&& func) {
        PTx { func(); }
    }


    /*
     * Allocator
     * Must be called from within a Mnemosyne transaction PTx {}
     */
    template <typename T, typename... Args>
    static T* alloc(Args&&... args) {
        void *addr = nullptr;
#ifdef USE_MNEMOSYNE
        addr = ::_ITM_pmalloc(sizeof(T));
#endif
        return new (addr) T(std::forward<Args>(args)...); // placement new
    }


    /*
     * De-allocator
     * Must be called from within a Mnemosyne transaction PTx {}
     */
    template<typename T>
    static void free(T* obj) {
#ifdef USE_MNEMOSYNE
        if (obj == nullptr) return;
        obj->~T();
        //::_ITM_pfree(obj);
#endif
    }

    /* Allocator for C methods (like memcached) */
    static void* pmalloc(size_t size) {
        void* ptr = nullptr;
#ifdef USE_MNEMOSYNE
        ptr = ::_ITM_pmalloc(size);
#endif
        return ptr;
    }


    /* De-allocator for C methods (like memcached) */
    static void pfree(void* ptr) {
#ifdef USE_MNEMOSYNE
        //::_ITM_pfree(ptr);
#endif
    }

    // Doesn't actually do any checking. That functionality exists only for RomulusLog and RomulusLR
    static bool consistency_check(void) {
        return true;
    }
};


/*
 * Definition of persist<> type
 * In Mnemosyne we don't do anything here, it's all up to the TM
 */
template<typename T>
struct persist {
    // Stores the actual value
    T val {};

    persist() { }

    persist(T initVal) : val{initVal} {}

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

    inline void pstore(T newVal) {
        val = newVal;
    }

    inline T pload() const {
        return val;
    }
};

} // end of mnemosyne namespace

#endif   // _MNEMOSYNE_PERSISTENCY_
