#ifndef DTX_H
#define DTX_H

#include <immintrin.h>
#include <vector>
#include <boost/any.hpp>
#include "../lockfreelist/lockfreelist.h"


#define FLUSH_ALIGN ((uintptr_t)64)
// Preprocessor parameter that determines whether or not to provide durability
#define USING_DURABLE_TXN 1


// #define USE_MMAP_FILE
#define ENABLE_PERSISTENCE


/*
FROM PMDK:
 * The x86 memory instructions are new enough that the compiler
 * intrinsic functions are not always available.  The intrinsic
 * functions are defined here in terms of asm statements for now.
 */
#define pmem_clflushopt(addr)  asm volatile(".byte 0x66; clflush %0" : "+m" (*(volatile char *)(addr)))    // clflushopt 

#define pmem_clwb(addr) asm volatile(".byte 0x66; xsaveopt %0" : "+m" (*(volatile char *)(addr)))  // clwb() 


#if defined ENABLE_READ_ONLY_OPT
    #define READ_ONLY_OPT_CODE( readOnlyCode ) readOnlyCode
    #define NO_READ_ONLY_OPT_CODE( noReadOnlyCode ) 
#else
    #define READ_ONLY_OPT_CODE( readOnlyCode ) 
    #define NO_READ_ONLY_OPT_CODE( noReadOnlyCode ) noReadOnlyCode
#endif



#if defined ENABLE_PERSISTENCE
    #define PERSIST_CODE( persistCode ) persistCode

    #define DTX_PERSIST_FLUSH( MEM_TO_PERSIST, MEM_LEN_TO_PERSIST ) (  DTX::PERSIST_FLUSH_ONLY(MEM_TO_PERSIST, MEM_LEN_TO_PERSIST) )

    #define DTX_PERSIST_BARRIER( ) (  DTX::PERSIST_BARRIER_ONLY() )
    
#else
    #define DTX_PERSIST_FLUSH( MEM_TO_PERSIST, MEM_LEN_TO_PERSIST )
    #define DTX_PERSIST_BARRIER( ) 
    #define PERSIST_CODE( persistCode )
#endif

/* 
 * A single entry in the undo log.
 * It holds a pointer to data that will be changed, along with the old data.
 */
template <typename T>
struct LogEntry
{
    T* ptr;
    T oldData;

    LogEntry(T* ptr, T oldData)
    {
        this.ptr = ptr;
        this.oldData = oldData;
    }
};

enum TxStatus
{
    TxStatus_ACTIVE,
    TxStatus_COMMITTED
};

/* 
 * A log containing all of the write instructions that will be executed by the current transaction.
 * Each UndoLog is owned by a single thread.
 */
struct UndoLog
{
    std::vector<LogEntry<boost::any>>* entries;
    TxStatus status;

    void Init();

    template <typename T>
    void Push(T* ptr, T oldData);

    void Uninit();
};

/* 
 * Class for durable transaction support.
 * 
 * To allow a thread to run durable transactions:
 *      INIT()
 * 
 * To begin a durable transaction:
 *      TX_BEGIN()
 * 
 * Within the transaction, change all write instructions (including CAS) as follows:
 *      Before:
 *          n->next = right;
 *      After:
 *          CREATE_UNDO_LOG_ENTRY(&n);
 *          n->next = right;
 *          PERSIST(&n);
 * 
 * To commit a durable transaction:
 *      TX_COMMIT()
 */
class DTX
{
public:

    static __thread UndoLog* log;

    /* 
    * Creates an undo log for the thread.
    * Should be called for each thread before it runs any transactions.
    */
    // static void INIT()
    // {
    //     log = new UndoLog();
    // }

    // /* 
    //  * Begins a transaction.
    //  */
    // static void TX_BEGIN()
    // {
    // #ifdef USING_DURABLE_TXN
    //     log->Init();
    //     log->status = TxStatus_ACTIVE;
    //     PERSIST(&(log->status), 1);
    // #endif
    // }

    // /* 
    //  * Commits a transaction.
    //  */
    // static void TX_COMMIT()
    // {
    // #ifdef USING_DURABLE_TXN
    //     log->status = TxStatus_COMMITTED;
    //     PERSIST(&(log->status), 1);
    //     log->Uninit();
    // #endif
    // }

    // /* 
    //  * Adds a new entry to the undo log and persists it.
    //  * Should be called before executing a write instruction in a transaction.
    //  */
    // template <typename T>
    // static void CREATE_UNDO_LOG_ENTRY(T* ptr)
    // {
    // #ifdef USING_DURABLE_TXN
    //     log->Push(ptr, *ptr);
    //     PERSIST(ptr, 1);
    // #endif
    // }

    /* 
     * A persistence barrier.
     * Inputs a pointer, and flushes the contents of that pointer, followed by an SFENCE.
     */
    // template <typename T>
    static void PERSIST(const void *addr, size_t len)
    {
    #ifdef USING_DURABLE_TXN

	uintptr_t uptr;

	/*
	 * Loop through cache-line-size (typically 64B) aligned chunks
	 * covering the given range.
	 */
	for (uptr = (uintptr_t)addr & ~(FLUSH_ALIGN - 1);
		uptr < (uintptr_t)addr + len * 100; uptr += FLUSH_ALIGN) 
        {
            _mm_clflush((char *)uptr);
        }
        _mm_sfence();
    #endif
    }

   

    static void PERSIST_FLUSH_ONLY(const void *addr, size_t len)
    {

	uintptr_t uptr;

	/*
	 * Loop through cache-line-size (typically 64B) aligned chunks
	 * covering the given range.
	 */
	for (uptr = (uintptr_t)addr & ~(FLUSH_ALIGN - 1);
		uptr < (uintptr_t)addr + len * 100; uptr += FLUSH_ALIGN) 
        {
            #if defined PWB_IS_CLFLUSH
                // printf("using PWB_IS_CLFLUSH\n\r");
                _mm_clflush((char *)uptr);
            #elif defined PWB_IS_CLFLUSHOPT
            // printf("using PWB_IS_CLFLUSHOPT\n\r");
                pmem_clflushopt((char *)uptr);
            #elif defined PWB_IS_CLWB
            // printf("using PWB_IS_CLWB\n\r");
                pmem_clwb((char *)uptr);
            #endif
        }
        

    




    }    

    static void PERSIST_BARRIER_ONLY()
    {
    #ifdef USING_DURABLE_TXN
        _mm_sfence();
    #endif
    }    

    /* 
     * TODO:
     * TX_ABORT()
     */

    /* 
     * TODO:
     * RECOVER()
     */
};


#endif /* end of include guard: DTX_H */