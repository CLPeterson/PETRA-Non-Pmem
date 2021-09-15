#ifndef ROMULUSSETADAPTOR_H
#define ROMULUSSETADAPTOR_H

#include <cmath>
#include <vector>


#include "../romulus/datastructures/TMLinkedListSet.hpp"
#include "../romulus/datastructures/TMHashMap.hpp"
#include "../romulus/datastructures/TMSkipList.hpp"
#include "../romulus/datastructures/TMMDListSet.hpp"


enum SetOpType
{
    FIND = 0,
    INSERT,
    DELETE
};

struct SetOperator
{
    uint8_t type;
    uint32_t key;
};

enum SetOpStatus
{
    LIVE = 0,
    COMMITTED,
    ABORTED
};

enum SetPersistStatus
{
    MAYBE = 0,
    IN_PROGRESS,
    PERSISTED,
};

typedef std::vector<SetOperator> SetOpArray;

template<typename T>
class SetAdaptor
{
};



template<>
class SetAdaptor<TMLinkedListSet<int>>
{
public:
    SetAdaptor(uint64_t cap, uint64_t threadCount, uint32_t transSize, uint32_t keyRange)
    {
        romulusSet = nullptr;
        TM_WRITE_TRANSACTION([&] () {
            romulusSet = TM_ALLOC<TMLinkedListSet<int>>();
        });        
    }

    ~SetAdaptor() {
        TM_WRITE_TRANSACTION([&] () {
            TM_FREE(romulusSet);
        });        
        printf("Total commit %u, abort (total/fake) %u/0\n", g_count_commit, g_count_abort);
    }

    void Init()
    {
        g_count_commit = 0;
        g_count_abort = 0;
        g_count_stm_abort = 0;
    }

    void Uninit(){
        // delete romulusSet;
    }

    bool ExecuteOps(const SetOpArray& ops)
    {
        bool ret = true;
        // TM_BEGIN_TRANSACTION();
        TM_WRITE_TRANSACTION([&] () {

     
        if(ret == true)
        {
            for(uint32_t i = 0; i < ops.size(); ++i)
            {
                uint32_t val = ops[i].key;
                if(ops[i].type == FIND)
                {
                    ret = romulusSet->contains(val);
                }
                else if(ops[i].type == INSERT)
                {
                    ret = romulusSet->add(val);
                }
                else
                {
                    ret = romulusSet->remove(val);
                }

                if(ret == false)
                {
                    //stm::restart();
                    // tx->tmabort(tx);
                    // std::cout << "Figure out how to abort!\n\r" << std::endl;
                    break;
                }
            }
        }
        // TM_END_TRANSACTION();
        });   

        if(ret)
        {
            __sync_fetch_and_add(&g_count_commit, 1);
        }
        else
        {
            __sync_fetch_and_add(&g_count_abort, 1);
        }
        // printf("commit %u, abort %u\n\r", g_count_commit, g_count_abort);

        return ret;

        // return m_list->ExecuteOps(desc);
    }

private:

    TMLinkedListSet<int>* romulusSet;

    uint32_t g_count_commit = 0;
    uint32_t g_count_abort = 0;  
    uint32_t g_count_stm_abort = 0;

};

template<>
class SetAdaptor<TMSkipList<int>>
{
public:
    SetAdaptor(uint64_t cap, uint64_t threadCount, uint32_t transSize, uint32_t keyRange)
    {
        romulusSkipList = nullptr;
        TM_WRITE_TRANSACTION([&] () {
            romulusSkipList = TM_ALLOC<TMSkipList<int>>();
        });        
    }

    ~SetAdaptor() {
        TM_WRITE_TRANSACTION([&] () {
            TM_FREE(romulusSkipList);
        });        
        printf("Total commit %u, abort (total/fake) %u/0\n", g_count_commit, g_count_abort);
    }

    void Init()
    {
        g_count_commit = 0;
        g_count_abort = 0;
        g_count_stm_abort = 0;
    }

    void Uninit(){
        // delete romulusSet;
    }

    bool ExecuteOps(const SetOpArray& ops)
    {
        bool ret = true;
        // TM_BEGIN_TRANSACTION();
        TM_WRITE_TRANSACTION([&] () {

     
        if(ret == true)
        {
            for(uint32_t i = 0; i < ops.size(); ++i)
            {
                uint32_t val = ops[i].key;
                if(ops[i].type == FIND)
                {
                    ret = romulusSkipList->contains(val);
                }
                else if(ops[i].type == INSERT)
                {
                    ret = romulusSkipList->add(val);
                }
                else
                {
                    ret = romulusSkipList->remove(val);
                }

                if(ret == false)
                {
                    //stm::restart();
                    // tx->tmabort(tx);
                    // std::cout << "Figure out how to abort!\n\r" << std::endl;
                    break;
                }
            }
        }
        // TM_END_TRANSACTION();
        });   

        if(ret)
        {
            __sync_fetch_and_add(&g_count_commit, 1);
        }
        else
        {
            __sync_fetch_and_add(&g_count_abort, 1);
        }
        // printf("commit %u, abort %u\n\r", g_count_commit, g_count_abort);

        return ret;

        // return m_list->ExecuteOps(desc);
    }

private:

    TMSkipList<int>* romulusSkipList;

    uint32_t g_count_commit = 0;
    uint32_t g_count_abort = 0;  
    uint32_t g_count_stm_abort = 0;

};

template<>
class SetAdaptor<TMMDListSet<int>>
{
public:
    SetAdaptor(uint64_t cap, uint64_t threadCount, uint32_t transSize, uint32_t keyRange)
    {
        romulusMDList = nullptr;
        TM_WRITE_TRANSACTION([&] () {
            romulusMDList = TM_ALLOC<TMMDListSet<int>>();
        });        
    }

    ~SetAdaptor() {
        TM_WRITE_TRANSACTION([&] () {
            TM_FREE(romulusMDList);
        });        
        printf("Total commit %u, abort (total/fake) %u/0\n", g_count_commit, g_count_abort);
    }

    void Init()
    {
        g_count_commit = 0;
        g_count_abort = 0;
        g_count_stm_abort = 0;
    }

    void Uninit(){
        // delete romulusSet;
    }

    bool ExecuteOps(const SetOpArray& ops)
    {
        bool ret = true;
        // TM_BEGIN_TRANSACTION();
        TM_WRITE_TRANSACTION([&] () {

     
        if(ret == true)
        {
            for(uint32_t i = 0; i < ops.size(); ++i)
            {
                uint32_t val = ops[i].key;
                if(ops[i].type == FIND)
                {
                    ret = romulusMDList->contains(val);
                }
                else if(ops[i].type == INSERT)
                {
                    ret = romulusMDList->add(val);
                }
                else
                {
                    ret = romulusMDList->remove(val);
                }

                if(ret == false)
                {
                    //stm::restart();
                    // tx->tmabort(tx);
                    // std::cout << "Figure out how to abort!\n\r" << std::endl;
                    break;
                }
            }
        }
        // TM_END_TRANSACTION();
        });   

        if(ret)
        {
            __sync_fetch_and_add(&g_count_commit, 1);
        }
        else
        {
            __sync_fetch_and_add(&g_count_abort, 1);
        }
        // printf("commit %u, abort %u\n\r", g_count_commit, g_count_abort);

        return ret;

        // return m_list->ExecuteOps(desc);
    }

private:

    TMMDListSet<int>* romulusMDList;

    uint32_t g_count_commit = 0;
    uint32_t g_count_abort = 0;  
    uint32_t g_count_stm_abort = 0;

};

template<typename T>
class MapAdaptor
{
};

enum MapOpType
{
    MAP_FIND = 0,
    MAP_INSERT,
    MAP_DELETE,
    MAP_UPDATE
};

struct MapOperator
{
    uint8_t type;
    uint32_t key;
    uint32_t value;
    uint32_t expected;
    uint32_t threadId;
};

enum MapOpStatus
{
    MAP_ACTIVE = 0,
    MAP_COMMITTED,
    MAP_ABORTED
};

typedef std::vector<MapOperator> MapOpArray;

template<>
class MapAdaptor<TMHashMap<int,int>>
{
public:


    MapAdaptor(uint64_t cap, uint64_t threadCount, uint32_t transSize, uint32_t keyRange)
    { 
        cap = std::max(cap,(uint64_t)keyRange);
        int mapHeadSize				=std::pow(2, ((int)std::ceil(std::log2(cap))));
        cap = mapHeadSize;// = cap;
        romulusMap = nullptr;
        TM_WRITE_TRANSACTION([&] () {
            romulusMap = TM_ALLOC<TMHashMap<int,int>>();
        });            


    }

    ~MapAdaptor() {
        TM_WRITE_TRANSACTION([&] () {
            TM_FREE(romulusMap);
        });        
        printf("Total commit %u, abort (total/fake) %u/0\n", g_count_commit, g_count_abort);
    }    

    void Init()
    {
        g_count_commit = 0;
        g_count_abort = 0;
        g_count_stm_abort = 0;
    }

    void Uninit(){
        // delete romulusMap;
    }

    bool ExecuteOps(const MapOpArray& ops, int threadId)
    {
        bool ret = true;
// #ifndef PMDK_PTM
//         TM_BEGIN_TRANSACTION();
// #else        
        TM_WRITE_TRANSACTION([&] () {
// #endif
            if(ret == true)
            {
                for(uint32_t i = 0; i < ops.size(); ++i)
                {
                    uint32_t val = ops[i].key;
                    if(ops[i].type == MAP_FIND)
                    {
                        ret = romulusMap->contains(val);
                    }
                    else if(ops[i].type == MAP_INSERT || ops[i].type == MAP_UPDATE)
                    {
                        ret = romulusMap->add(val);
                    }
                    else
                    {
                        ret = romulusMap->remove(val);
                    }

                    if(ret == false)
                    {
                        //stm::restart();
                        // tx->tmabort(tx);
                        // std::cout << "Figure out how to abort: " << g_count_abort << std::endl;
                        break;
                    }
                }
            }
        // TM_END_TRANSACTION();
// #ifndef PMDK_PTM        
        // TM_END_TRANSACTION();
// #else
   }); 
// #endif         

        if(ret)
        {
            __sync_fetch_and_add(&g_count_commit, 1);
        }
        else
        {
            __sync_fetch_and_add(&g_count_abort, 1);
        }
        // printf("commit %u, abort %u\n\r", g_count_commit, g_count_abort);

        return ret;
    }

private:
    TMHashMap<int,int>* romulusMap;

    uint32_t g_count_commit = 0;
    uint32_t g_count_abort = 0;  
    uint32_t g_count_stm_abort = 0;  

};




#endif /* end of include guard: ROMULUSSETADAPTOR_H */
