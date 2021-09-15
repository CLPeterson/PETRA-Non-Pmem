#ifndef ALLOCATOR_H
#define ALLOCATOR_H


/***
 * MMAP HEADERS
 * **/
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h> /* For O_* constants */
#include <string.h>
#include <errno.h>




#include <cstdint>
#include <malloc.h>
#include <atomic>
#include <iostream>
#include "assert.h"

#ifndef USE_MMAP_ALLOCATOR
    #ifndef USE_PMEM_ALLOCATOR
        #define USE_DRAM_ALLOCATOR
    #endif
#endif

#if defined USE_DRAM_ALLOCATOR
    #define ALLOCATOR_DRAM_ONLY_CODE( DRAMCode ) DRAMCode
    #define ALLOCATOR_MMAP_ONLY_CODE( MMAPCode ) 
    #define ALLOCATOR_PMEM_ONLY_CODE( PMEMCode )   
    #define ALLOCATOR_PERSISTABLE_ONLY_CODE( PersistCode )     
#elif defined USE_MMAP_ALLOCATOR
    #define ALLOCATOR_DRAM_ONLY_CODE( DRAMCode ) 
    #define ALLOCATOR_MMAP_ONLY_CODE( MMAPCode ) MMAPCode
    #define ALLOCATOR_PMEM_ONLY_CODE( PMEMCode )
    #define ALLOCATOR_PERSISTABLE_ONLY_CODE( PersistCode ) PersistCode
#elif defined USE_PMEM_ALLOCATOR
    #define ALLOCATOR_DRAM_ONLY_CODE( DRAMCode ) 
    #define ALLOCATOR_MMAP_ONLY_CODE( MMAPCode )
    #define ALLOCATOR_PMEM_ONLY_CODE( PMEMCode ) PMEMCode
    #define ALLOCATOR_PERSISTABLE_ONLY_CODE( PersistCode ) PersistCode

    #include <libpmem.h>
    #include <immintrin.h>
    #define PMEM_ROOT_PATH "/mnt/pmem0/"

#else
    #define ALLOCATOR_DRAM_ONLY_CODE( DRAMCode )
    #define ALLOCATOR_MMAP_ONLY_CODE( MMAPCode ) 
    #define ALLOCATOR_PMEM_ONLY_CODE( PMEMCode )   
    #define ALLOCATOR_PERSISTABLE_ONLY_CODE( PersistCode )
#endif




template<typename DataType>
class Allocator 
{
public:
    Allocator(uint64_t totalBytes, uint64_t threadCount, uint64_t typeSize)
    {
        m_totalBytes = totalBytes;
        m_threadCount = threadCount;
        m_typeSize = typeSize;
        m_ticket = 0;
        m_pool = (char*)memalign(m_typeSize, totalBytes);
        isReloadedMem = false;

        ASSERT(m_pool, "Memory pool initialization failed.");
    }

    Allocator()
    {
        // logger.pmem_durableds_dlog("empty constructor was called! siseof(DataType)=", sizeof(DataType),"\n\r");
    }  

    ~Allocator()
    {
        // std::cout << "~Allocator() is called" << std::endl;
        ALLOCATOR_MMAP_ONLY_CODE(
            close(shm_fd);
        )

        ALLOCATOR_DRAM_ONLY_CODE(
            free(m_pool);
        )
    }

    bool load_existing_mem_mmap_allocator(const char* name, uint64_t threadCount, uint64_t typeSize)
    {
        bool new_mem = true;
        isReloadedMem = true;
        // isMemMapped = true;
        
    	mode_t perms = S_IRUSR | S_IWUSR;

        int current_size = 0;

        int flags = O_RDWR;
        int fd = shm_open(name, flags, perms);
        if(fd == -1) {
            std::cout << "ERROR: shm_open with name " << name << " error: " << errno << ": " << strerror(errno) << std::endl;
            return false;
        }else{
            struct stat sb;
            fstat( fd , &sb );
            current_size = sb.st_size;
        }

        m_pool = (char*)mmap(NULL, current_size, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, 0);

        if(MAP_FAILED == m_pool) {
            std::cout << "ERROR: mmap error! (" << errno << "): " <<  strerror(errno) << ", size="<<  current_size <<  ", name=" <<  name <<  "requested size=" <<  current_size <<  std::endl;
            return false;
        }
        shm_name = name;
        shm_fd = fd;  

        m_totalBytes = current_size;
        m_threadCount = threadCount;
        m_typeSize = typeSize;
        m_ticket = 0;
    }

    void buildFreeIndexVector(const char * rootMemName)
    {
        const char* freeIndexFileNamePostFix = "_freeIndex";
        char* freeIndexVectorName = new char[strlen(rootMemName) + strlen(freeIndexFileNamePostFix) + 1];
        sprintf(freeIndexVectorName, "%s%s", rootMemName, freeIndexFileNamePostFix);
        int current_size = m_threadCount * sizeof(uint64_t);
        ALLOCATOR_MMAP_ONLY_CODE
        (
            mode_t perms = S_IRUSR | S_IWUSR;
            int flags = O_RDWR | O_CREAT | O_EXCL;
            int fd = shm_open(freeIndexVectorName, flags, perms);
            
            if(fd == -1) {
                if(EEXIST == errno){
                    // std::cout << "memory with name " << freeIndexVectorName << " already exists" std::endl;

                    perms = S_IRUSR | S_IWUSR;

                    current_size = 0;

                    flags = O_RDWR;
                    fd = shm_open(freeIndexVectorName, flags, perms);
                    if(fd == -1) {
                        std::cout << "ERROR: shm_open with name " << freeIndexVectorName << " error: "  << errno  << ": "  << strerror(errno) << std::endl;
                        return;
                    }else{
                        struct stat sb;
                        fstat( fd , &sb );
                        current_size = sb.st_size;
                    }

                    mg_freeIndex = (uint64_t*)mmap(NULL, current_size, PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);

                    if(MAP_FAILED == mg_freeIndex) {
                        std::cout << "ERROR: mmap error! (" << errno << "): " << strerror(errno) << ", size=" << current_size << ", name=" << freeIndexVectorName << "requested size=" << current_size << std::endl;
                        return;
                    }

                }else {
                    std::cout << "ERROR: shm_open with name " << freeIndexVectorName << " error: " << errno << ": " << strerror(errno) << std::endl;
                }
            }else {
                // logger.pmem_durableds_dlog("creating memory with name ", freeIndexVectorName);
                if(ftruncate(fd, current_size) == -1) {
                    std::cout << "ERROR: ftruncate error: " << errno << ": " << strerror(errno) << std::endl;
                }            
        

                mg_freeIndex = (uint64_t*)mmap(NULL, current_size, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, 0);

                if(MAP_FAILED == mg_freeIndex) {
                    std::cout << "ERROR: mmap error! (" << errno << "): " << strerror(errno) << ", size=" << current_size << ", name=" << freeIndexVectorName << "requested size=" << current_size << std::endl;
                }
                memset(mg_freeIndex, 0, current_size);                
            }
        )
        ALLOCATOR_PMEM_ONLY_CODE
        (
            size_t mapped_len;
            int is_pmem;
            int flags = PMEM_FILE_CREATE | PMEM_FILE_EXCL;
            mode_t mode = 0666;
            if(isReloadedMem) {
                flags = 0;
                current_size = 0;
            }
            mg_freeIndex = (uint64_t* )pmem_map_file(freeIndexVectorName, current_size, flags, mode, &mapped_len, &is_pmem);
            if(NULL == mg_freeIndex) {
                std::cout << "ERROR: pmem_map_file error! (" << errno << "): " << strerror(errno) << ", size=" << mapped_len << ", name=" << freeIndexVectorName << " requested size=" << current_size << std::endl;
                exit(1);
            }
            if(!is_pmem) {
                std::cout << "ERROR: memory created by pmem_map_file is not in persistent memory , size=" << mapped_len << ", name=" << freeIndexVectorName << "requested size=" << current_size << std::endl;
                exit(1);
            }            

        )        

    }

    Allocator(const char* name, uint64_t totalBytes, uint64_t threadCount, uint64_t typeSize)
    {
        prev_m_pool = nullptr;
           
        // init(name, totalBytes);
        // logger.pmem_durableds_dlog("Allocator was called with name ", name, ", totalBytes=", totalBytes, ", typeSize=", typeSize);
        ALLOCATOR_MMAP_ONLY_CODE
        (
            isReloadedMem = false;
            bool new_mem = true;
            mode_t perms = S_IRUSR | S_IWUSR;
            int flags = O_RDWR | O_CREAT | O_EXCL;

            int fd = shm_open(name, flags, perms);
            uint64_t current_size = totalBytes;

            if(fd == -1) {
                if(EEXIST == errno){
                    // std::cout << "DEBUG: memory with name " << name << " already exists" << std::endl;
                    load_existing_mem_mmap_allocator(name, threadCount, typeSize);
                }else {
                    std::cout << "ERROR: shm_open with name " << name << " error: " << errno << ": " <<  strerror(errno) << std::endl;
                }

            }else {
                // std::cout << "DEBUG:creating memory with name " << name << std::endl;
                if(new_mem)  {
                    if(ftruncate(fd, current_size) == -1) {
                        std::cout << "ERROR: ftruncate error: " << errno << ": " << strerror(errno) << std::endl;
                    }            
                }

                m_pool = (char*)mmap(NULL, current_size, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, 0);

                if(MAP_FAILED == m_pool) {
                    std::cout << "ERROR: mmap error! (" << errno << "): " << strerror(errno) << ", size=" << current_size << ", name=" << name << "requested size=" << current_size << std::endl;
                }
                if(new_mem)
                    memset(m_pool, 0, current_size);
                shm_name = name;
                shm_fd = fd;  

                m_totalBytes = totalBytes;
                m_threadCount = threadCount;
                m_typeSize = typeSize;
                m_ticket = 0;
            }
            buildFreeIndexVector(shm_name);
            
            // std::cout << "DEBUG: Allocator was completed with name " << name << ", m_pool=" << m_pool << std::endl;
            ASSERT(m_pool, "Memory pool initialization failed.");
            // print();
        )  

        ALLOCATOR_PMEM_ONLY_CODE
        (
            isReloadedMem = false;
            size_t mapped_len;
            int is_pmem;
            int flags = PMEM_FILE_CREATE | PMEM_FILE_EXCL;
            mode_t mode = 0666;
            char *trans_pmem_name = new char[strlen(PMEM_ROOT_PATH) + strlen(name) + 1];
            char const  *prefix = PMEM_ROOT_PATH;
            sprintf(trans_pmem_name,  "%s%s", prefix,  name);
            shm_name = trans_pmem_name;
            m_pool = (char *)pmem_map_file(trans_pmem_name, totalBytes, flags, mode, &mapped_len, &is_pmem);
            
            if(NULL == m_pool) {
                if(EEXIST == errno) {
                    isReloadedMem = true;
                    flags = PMEM_FILE_CREATE;
                    m_pool = (char *)pmem_map_file(trans_pmem_name, totalBytes, flags, mode, &mapped_len, &is_pmem);
                    if(NULL == m_pool) {
                        std::cout << "ERROR: pmem_map_file error1! (" << errno << "): " << strerror(errno) << ", size=" << mapped_len << ", name=" << trans_pmem_name << "requested size=" << totalBytes << std::endl;
                        exit(1);                        
                    }
                }else {
                    std::cout << "ERROR: pmem_map_file error2! (" << errno << "): " << strerror(errno) << ", size=" << mapped_len << ", name=" << trans_pmem_name << "requested size=" << totalBytes << std::endl;
                    exit(1);
                }

            }
            if(!is_pmem) {
                std::cout << "ERROR: memory created by pmem_map_file is not in persistent memory , size=" << mapped_len << ", name=" << trans_pmem_name << "requested size=" << totalBytes << std::endl;
                exit(1);
            }else {
                
                m_totalBytes = totalBytes;
                m_threadCount = threadCount;
                m_typeSize = typeSize;
                m_ticket = 0;
                buildFreeIndexVector(trans_pmem_name);
            }
        )
        prev_m_pool = m_pool;    
    } 


    void reload_pmem(const char* name)
    {
ALLOCATOR_PMEM_ONLY_CODE
(        
        isReloadedMem = true;
    	mode_t mode = 0666;

        int current_size = 0;

        int flags = 0;

        size_t mapped_len;
        int is_pmem;
        
        char  *trans_pmem_name = new char[strlen(PMEM_ROOT_PATH) + strlen(name) + 1];
        char const  *prefix = PMEM_ROOT_PATH;
        sprintf(trans_pmem_name,  "%s%s", prefix,  name);
        shm_name = trans_pmem_name;

        m_pool = (char *)pmem_map_file(trans_pmem_name, 0, flags, mode, &mapped_len, &is_pmem);

        if(NULL == m_pool) {
            std::cout << "ERROR: pmem_map_file error! (" << errno << "): " << strerror(errno) << ", size=" << mapped_len << ", name=" << trans_pmem_name  << std::endl;
            exit(1);
        }
        if(!is_pmem) {
            std::cout << "ERROR: memory created by pmem_map_file is not in persistent memory , size=" << mapped_len << ", name=" << trans_pmem_name << std::endl;
            exit(1);
        }else {
            
            // m_totalBytes = totalBytes;
            // m_threadCount = threadCount;
            // m_typeSize = typeSize;
            m_ticket = 0;
            buildFreeIndexVector(trans_pmem_name);
        }                

)


    }    

    //Every thread need to call init once before any allocation
    void Init()
    {
        threadId = __sync_fetch_and_add(&m_ticket, 1);
        ASSERT(threadId < m_threadCount, "ThreadId specified should be smaller than thread count.");

        m_base = m_pool + threadId * m_totalBytes / m_threadCount;

        ALLOCATOR_DRAM_ONLY_CODE
        (       
            m_freeIndex = 0;
        )

        ALLOCATOR_PERSISTABLE_ONLY_CODE
        (
            // std::cout << "Init(): " << isReloadedMem << std::endl;
            if(isReloadedMem) {
                m_freeIndex = mg_freeIndex[threadId];
            } else {
                m_freeIndex = 0;
                mg_freeIndex[threadId] = 0;
            }
        )
    }

    void reset()
    {
        m_ticket = 0;
        // std::cout << "reset m_ticket "<< m_ticket << std::endl;
    }

    void reInit()
    {
        // std::cout << "reInit m_ticket "<< m_ticket << std::endl;
        threadId = __sync_fetch_and_add(&m_ticket, 1);
        ASSERT(threadId < m_threadCount, "ThreadId specified should be smaller than thread count.");

        m_base = m_pool + threadId * m_totalBytes / m_threadCount;

        ALLOCATOR_DRAM_ONLY_CODE
        (       
            m_freeIndex = 0;
        )

        ALLOCATOR_PERSISTABLE_ONLY_CODE
        (
            // std::cout << "reInit: ALLOCATOR_PERSISTABLE_ONLY_CODE " << std::endl;
            // std::cout << "Init(): " << isReloadedMem << std::endl;
            // if(isReloadedMem) {
                m_freeIndex = mg_freeIndex[threadId];
            // } else {
                // m_freeIndex = 0;
                // mg_freeIndex[threadId] = 0;
            // }
        )
    }    

    void Uninit()
    { }

// ALLOCATOR_PERSISTABLE_ONLY_CODE
// (
    DataType* getFirst()
    {
        ASSERT(m_base, "out of capacity.");
        char* ret = m_base;
        // std::cout << "getFirst(): " << ret << std::endl;
        return (DataType*)ret;
    }

    DataType* getNext(DataType* p)
    {
        ASSERT((char*)p + m_typeSize < m_base + m_totalBytes / m_threadCount, "out of capacity.");
        ASSERT((char*)p + m_typeSize < m_base + m_freeIndex, "end of used area");
        // printf("first=%p, second=%p\n\r", (char*)p + m_typeSize, m_base + m_freeIndex);
        // std::cout << "getNext(): " << std::endl;
        if((char*)p + m_typeSize >= m_base + m_freeIndex)
            return nullptr;
        char* ret = (char*)p + m_typeSize;
        return (DataType*)ret;
    }

    DataType* getNextForThread(DataType* p, int t)
    {
        char* ret_base = m_pool + t * m_totalBytes / m_threadCount;
        // std::cout << "getNextForThread(): " << ret_base << std::endl;
        // std::cout << mg_freeIndex[t] << std::endl;
        if((char*)p + m_typeSize >= ret_base + mg_freeIndex[t])
            return nullptr;
        char* ret = (char*)p + m_typeSize;
        // printf("%p <> %p <> %d\n\r", ret, (ret_base + mg_freeIndex[t]), mg_freeIndex[t]);
        // std::cout <<  << " <> " <<  << " " << ret_base + mg_freeIndex[t] << std::endl;
        return (DataType*)ret;            
 
    }

    DataType* getFirstForThread(int t)
    {
        char* ret_base = m_pool + t * m_totalBytes / m_threadCount;
        // std::cout << "getFirstForThread(): " << ret_base << std::endl;
        return (DataType*)ret_base;
    }

    DataType* getNewPointer(DataType* oldp)
    {
        if(prev_m_pool == nullptr || oldp == nullptr)
            return oldp;
    //     ASSERT((char*)p + m_typeSize < m_base + m_totalBytes / m_threadCount, "out of capacity.");
    //     ASSERT((char*)p + m_typeSize < m_base + m_freeIndex, "end of used area");
        // printf("first=%p, second=%p\n\r", (char*)p + m_typeSize, m_base + m_freeIndex);
        // std::cout << "getNext(): " << std::endl;
        uint64_t diff = (char *)oldp - prev_m_pool;
        char* ret = m_pool + diff;
        return (DataType*)ret;
    }    

// )

    DataType* Alloc()
    {
        // printf("m_freeIndex=%lu, m_totalBytes / m_threadCount=%lu\n\r", m_freeIndex, m_totalBytes / m_threadCount);
        // print();
        if(m_freeIndex >= ((m_totalBytes / m_threadCount) - 2 * m_typeSize)) {
            printf("m_freeIndex=%lu, m_totalBytes / m_threadCount=%lu\n\r", m_freeIndex, m_totalBytes / m_threadCount);
            print();
        }
            
        

        ASSERT(m_freeIndex < m_totalBytes / m_threadCount, "out of capacity.");
        char* ret = m_base + m_freeIndex;
        m_freeIndex += m_typeSize;
        ALLOCATOR_PERSISTABLE_ONLY_CODE
        (
            // if(m_typeSize == 24) {
            //     std::cout << "Alloc(): " <<  allocCounter++ <<  std::endl;
            //     print();
            // }
                
            mg_freeIndex[threadId] = m_freeIndex;
        )
                
        return (DataType*)ret;
    }

    void print()
    {
        printf("%s\n", shm_name);
        printf("\tm_pool=%p\n", m_pool);
        printf("\tm_totalBytes=%lu\n", m_totalBytes);
        printf("\tm_threadCount=%lu\n", m_threadCount);
        printf("\tthreadId=%lu\n", threadId);
        printf("\tm_ticket=%lu\n", m_ticket);
        printf("\tm_typeSize=%lu\n", m_typeSize);
        printf("\tm_base=%p\n", m_base);
        printf("\tm_freeIndex=%lu\n\r", m_freeIndex);        
    }    

private:
    bool isReloadedMem;
    const char* shm_name;
    int shm_fd;


    char* m_pool;
    uint64_t m_totalBytes;      //number of elements T in the pool
    uint64_t m_threadCount;
    uint64_t m_ticket;
    uint64_t m_typeSize;
    char* prev_m_pool;

    uint64_t* mg_freeIndex;

    static __thread uint64_t threadId;
    static __thread char* m_base;
    static __thread uint64_t m_freeIndex;
};

template<typename T>
__thread char* Allocator<T>::m_base;

template<typename T>
__thread uint64_t Allocator<T>::m_freeIndex;

template<typename T>
__thread uint64_t Allocator<T>::threadId;

#endif /* end of include guard: ALLOCATOR_H */
