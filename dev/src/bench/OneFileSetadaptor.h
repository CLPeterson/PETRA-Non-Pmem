#ifndef ROMULUSSETADAPTOR_H
#define ROMULUSSETADAPTOR_H

#include <cmath>
#include <vector>


#include "../OneFile/pdatastructures/TMLinkedListSet.hpp"
#include "../OneFile/ptms/OneFilePTMLF.hpp"
#include "../OneFile/pdatastructures/TMHashMap.hpp"
#include "../OneFile/pdatastructures/TMSkipList.hpp"
#include "../OneFile/pdatastructures/OFMDListSet.hpp"


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
class SetAdaptor<TMLinkedListSet<int,poflf::OneFileLF,poflf::tmtype>>
{
public:
    SetAdaptor(uint64_t cap, uint64_t threadCount, uint32_t transSize, uint32_t keyRange)
    {
        onefileSet = nullptr;
        poflf::OneFileLF::template updateTx<bool>([&] () {
            onefileSet = poflf::OneFileLF::template tmNew<TMLinkedListSet<int,poflf::OneFileLF,poflf::tmtype>>();
            return true;
        });

    }

    ~SetAdaptor() {

        poflf::OneFileLF::template updateTx<bool>([=] () {
            poflf::OneFileLF::tmDelete(onefileSet);
            return true;
        });
  
        printf("Total commit %u, abort (total/fake) %u/0\n", g_count_commit, g_count_abort);

		FILE* pfile = fopen("output.txt", "a"); //CORRECTNESS ANNOTATIONS

    	fprintf(pfile, "%u ", g_count_commit); //CORRECTNESS ANNOTATIONS

    	fclose(pfile); //CORRECTNESS ANNOTATIONS
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

        ret = poflf::OneFileLF::template updateTx<bool>([=] () {
                bool ret = true;
        // TM_WRITE_TRANSACTION([&] () {

            for(uint32_t i = 0; i < ops.size(); ++i)
            {
                uint32_t val = ops[i].key;
                if(ops[i].type == FIND)
                {
                    ret = onefileSet->contains(val);
                }
                else if(ops[i].type == INSERT)
                {
                    ret = onefileSet->add(val);
                }
                else
                {
                    ret = onefileSet->remove(val);
                }

                if(ret == false)
                {
                    //stm::restart();
                    // tx->tmabort(tx);
                    // std::cout << "Figure out how to abort!\n\r" << std::endl;
                    break;
                }
            }
            return ret;
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

    TMLinkedListSet<int,poflf::OneFileLF,poflf::tmtype>* onefileSet;

    uint32_t g_count_commit = 0;
    uint32_t g_count_abort = 0;  
    uint32_t g_count_stm_abort = 0;

};


#define NUM_LEVELS 20

template<>
class SetAdaptor<TMSkipList<int,poflf::OneFileLF,poflf::tmtype>>
{
public:
    SetAdaptor(uint64_t cap, uint64_t threadCount, uint32_t transSize, uint32_t keyRange)
    { 

        onefileSkipList = nullptr;
        poflf::OneFileLF::template updateTx<bool>([&] () {
            onefileSkipList = poflf::OneFileLF::template tmNew<TMSkipList<int,poflf::OneFileLF,poflf::tmtype>>();
            return true;
        });
    }

    ~SetAdaptor()
    {

        // poflf::OneFileLF::template updateTx<bool>([=] () {
        //     poflf::OneFileLF::tmDelete(onefileSkipList);
        //     return true;
        // });
  
        printf("Total commit %u, abort (total/fake) %u/0\n", g_count_commit, g_count_abort);
    }

    void Init()
    {
        g_count_commit = 0;
        g_count_abort = 0;
        g_count_stm_abort = 0;
    }

    void Uninit()
    {
        // destroy_transskip_subsystem(); 
    }

    bool ExecuteOps(const SetOpArray& ops)
    {
        //TransList::Desc* desc = m_list.AllocateDesc(ops.size());
bool ret = true;
        // TM_BEGIN_TRANSACTION();

        ret = poflf::OneFileLF::template updateTx<bool>([=] () {
                bool ret = true;
        // TM_WRITE_TRANSACTION([&] () {

            for(uint32_t i = 0; i < ops.size(); ++i)
            {
                uint32_t val = ops[i].key;
                if(ops[i].type == FIND)
                {
                    ret = onefileSkipList->contains(val);
                }
                else if(ops[i].type == INSERT)
                {
                    ret = onefileSkipList->add(val);
                }
                else
                {
                    ret = onefileSkipList->remove(val);
                }

                if(ret == false)
                {
                    //stm::restart();
                    // tx->tmabort(tx);
                    // std::cout << "Figure out how to abort!\n\r" << std::endl;
                    break;
                }
            }
            return ret;
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
    }

private:

    TMSkipList<int,poflf::OneFileLF,poflf::tmtype>* onefileSkipList;
    uint32_t g_count_commit = 0;
    uint32_t g_count_abort = 0;  
    uint32_t g_count_stm_abort = 0;    
};


template<>
class SetAdaptor<OFMDListSet<int,poflf::OneFileLF,poflf::tmtype>>
{
public:
    SetAdaptor(uint64_t cap, uint64_t threadCount, uint32_t transSize, uint32_t keyRange)
    { 

        onefileMDList = nullptr;
        poflf::OneFileLF::template updateTx<bool>([&] () {
            onefileMDList = poflf::OneFileLF::template tmNew<OFMDListSet<int,poflf::OneFileLF,poflf::tmtype>>();
            return true;
        });
    }

    ~SetAdaptor()
    {
        // poflf::OneFileLF::template updateTx<bool>([=] () {
        //     poflf::OneFileLF::tmDelete(onefileMDList);
        //     return true;
        // });
  
        printf("Total commit %u, abort (total/fake) %u/0\n", g_count_commit, g_count_abort);
    }

    void Init()
    {
        g_count_commit = 0;
        g_count_abort = 0;
        g_count_stm_abort = 0;
    }

    void Uninit()
    {
        // destroy_transskip_subsystem(); 
    }

    bool ExecuteOps(const SetOpArray& ops)
    {
        //TransList::Desc* desc = m_list.AllocateDesc(ops.size());
bool ret = true;
        // TM_BEGIN_TRANSACTION();

        ret = poflf::OneFileLF::template updateTx<bool>([=] () {
                bool ret = true;
        // TM_WRITE_TRANSACTION([&] () {

            for(uint32_t i = 0; i < ops.size(); ++i)
            {
                uint32_t val = ops[i].key;
                if(ops[i].type == FIND)
                {
                    ret = onefileMDList->contains(val);
                }
                else if(ops[i].type == INSERT)
                {
                    ret = onefileMDList->add(val);
                }
                else
                {
                    ret = onefileMDList->remove(val);
                }

                if(ret == false)
                {
                    //stm::restart();
                    // tx->tmabort(tx);
                    // std::cout << "Figure out how to abort!\n\r" << std::endl;
                    break;
                }
            }
            return ret;
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
    }

private:

    OFMDListSet<int,poflf::OneFileLF,poflf::tmtype>* onefileMDList;
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
class MapAdaptor<TMHashMap<int,int, poflf::OneFileLF,poflf::tmtype>>
{
public:


    MapAdaptor(uint64_t cap, uint64_t threadCount, uint32_t transSize, uint32_t keyRange)
    { 
        cap = std::max(cap,(uint64_t)keyRange);
        int mapHeadSize				=std::pow(2, ((int)std::ceil(std::log2(cap))));
        cap = mapHeadSize;// = cap;
        onefileMap = nullptr;

        poflf::OneFileLF::template updateTx<bool>([&] () {
            onefileMap = poflf::OneFileLF::template tmNew<TMHashMap<int,int, poflf::OneFileLF,poflf::tmtype>>();
            return true;
        });          


    }

    ~MapAdaptor() {


        poflf::OneFileLF::template updateTx<bool>([=] () {
            poflf::OneFileLF::tmDelete(onefileMap);
            return true;
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
        ret = poflf::OneFileLF::template updateTx<bool>([=] () {
                bool ret = true;
            if(ret == true)
            {
                for(uint32_t i = 0; i < ops.size(); ++i)
                {
                    uint32_t val = ops[i].key;
                    if(ops[i].type == MAP_FIND)
                    {
                        ret = onefileMap->contains(val);
                    }
                    else if(ops[i].type == MAP_INSERT || ops[i].type == MAP_UPDATE)
                    {
                        ret = onefileMap->add(val);
                    }
                    else
                    {
                        ret = onefileMap->remove(val);
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
            return ret;
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
    }

private:
    TMHashMap<int,int, poflf::OneFileLF,poflf::tmtype>* onefileMap;

    uint32_t g_count_commit = 0;
    uint32_t g_count_abort = 0;  
    uint32_t g_count_stm_abort = 0;  

};




#endif /* end of include guard: ROMULUSSETADAPTOR_H */
