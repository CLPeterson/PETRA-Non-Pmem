//------------------------------------------------------------------------------
// 
//     Testing different priority queues
//
//------------------------------------------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <array>
#include <set>

#include <mutex>
#include <boost/random.hpp>
#include <sched.h>

#include <thread>
#include "../common/timehelper.h"
#include "../common/threadbarrier.h"
#include "RomulusSetadaptor.h"


template<typename T>
void WorkThread(uint32_t numThread, int threadId, uint32_t testSize, uint32_t tranSize, uint32_t keyRange, uint32_t insertion, uint32_t deletion, ThreadBarrier& barrier,  T& set)
{
    //set affinity for each thread
    cpu_set_t cpu = {{0}};
    CPU_SET(threadId, &cpu);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpu);

    double startTime = Time::GetWallTime();

    boost::mt19937 randomGenKey;
    boost::mt19937 randomGenOp;
    randomGenKey.seed(startTime + threadId);
    randomGenOp.seed(startTime + threadId + 1000);
    boost::uniform_int<uint32_t> randomDistKey(1, keyRange);
    boost::uniform_int<uint32_t> randomDistOp(1, 100);
    
    set.Init();

    barrier.Wait();
    
    SetOpArray ops(tranSize);

    for(unsigned int i = 0; i < testSize; ++i)
    {
        for(uint32_t t = 0; t < tranSize; ++t)
        {
            uint32_t op_dist = randomDistOp(randomGenOp);
            ops[t].type = op_dist <= insertion ? INSERT : op_dist <= insertion + deletion ? DELETE : FIND;
            ops[t].key  = randomDistKey(randomGenKey);
        }

        set.ExecuteOps(ops);
    }
}

template<typename T>
void MapWorkThread(uint32_t numThread, int threadId, uint32_t testSize, uint32_t tranSize, uint32_t keyRange, uint32_t insertion, uint32_t deletion, uint32_t update, ThreadBarrier& barrier,  T& map)
{
    //set affinity for each thread
    cpu_set_t cpu = {{0}};
    CPU_SET(threadId, &cpu);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpu);

    double startTime = Time::GetWallTime();

    boost::mt19937 randomGenKey;
    boost::mt19937 randomGenOp;
    randomGenKey.seed(startTime + threadId);
    randomGenOp.seed(startTime + threadId + 1000);
    boost::uniform_int<uint32_t> randomDistKey(1, keyRange);
    boost::uniform_int<uint32_t> randomDistOp(1, 100);
    
    map.Init();

    barrier.Wait();
    
    MapOpArray ops(tranSize);

    for(unsigned int i = 0; i < testSize; ++i)
    {
        for(uint32_t t = 0; t < tranSize; ++t)
        {
            uint32_t op_dist = randomDistOp(randomGenOp);
            //ops[t].type = op_dist <= insertion ? INSERT : op_dist <= insertion + deletion ? DELETE : FIND;
            if(op_dist <= insertion)
            {
                ops[t].type = MAP_INSERT;
                ops[t].value = randomDistKey(randomGenKey) + 1;
            }
            else if(op_dist <= insertion + deletion)
            {
                ops[t].type = MAP_DELETE;
                ops[t].value = 0;
            }
            else if(op_dist <= insertion + deletion + update)
            {
                ops[t].type = MAP_UPDATE;
                ops[t].value = randomDistKey(randomGenKey) + 1;
            }
            else
            {
                ops[t].type = MAP_FIND;
                ops[t].value = 0;
                // std::cout << threadId << ":" << i << std::endl;
            }

            ops[t].key  = randomDistKey(randomGenKey) + 1;
        }

        //std::vector<VALUE> toR;
        // std::cout << "going to execute  transaction " << i << std::endl;
        map.ExecuteOps(ops, threadId);//, toR);

    }

    // map.Uninit();
}


template<typename T>
void Tester(uint32_t numThread, uint32_t testSize, uint32_t tranSize, uint32_t keyRange, uint32_t insertion, uint32_t deletion,  SetAdaptor<T>& set)
{
    std::vector<std::thread> thread(numThread);
    ThreadBarrier barrier(numThread + 1);

    double startTime = Time::GetWallTime();
    boost::mt19937 randomGen;
    randomGen.seed(startTime - 10);
    boost::uniform_int<uint32_t> randomDist(1, keyRange);

    set.Init();

    SetOpArray ops(1);

    for(unsigned int i = 0; i < keyRange; ++i)
    {
        ops[0].type = INSERT;
        ops[0].key  = randomDist(randomGen);
        set.ExecuteOps(ops);
    }

    //Create joinable threads
    for (unsigned i = 0; i < numThread; i++) 
    {
        thread[i] = std::thread(WorkThread<SetAdaptor<T> >, numThread, i + 1, testSize, tranSize, keyRange, insertion, deletion, std::ref(barrier), std::ref(set));
    }


    barrier.Wait();
    // static int jcounter = 1;

    {
        ScopedTimer timer(true);

        //Wait for the threads to finish
        for (unsigned i = 0; i < thread.size(); i++) 
        {
            thread[i].join();
        }
    }

    set.Uninit();
}

template<typename T>
void MapTester(uint32_t numThread, uint32_t testSize, uint32_t tranSize, uint32_t keyRange, uint32_t insertion, uint32_t deletion, uint32_t update, MapAdaptor<T>& map)
{
    std::vector<std::thread> thread(numThread);
    ThreadBarrier barrier(numThread + 1);

    double startTime = Time::GetWallTime();
    boost::mt19937 randomGen;
    randomGen.seed(startTime - 10);
    boost::uniform_int<uint32_t> randomDist(1, keyRange);

    map.Init();

    MapOpArray ops(1);

    for(unsigned int i = 0; i < keyRange; ++i)
    {
        //std::vector<VALUE> toR;
        ops[0].type = INSERT;
        ops[0].key  = randomDist(randomGen) + 1;
        ops[0].value = randomDist(randomGen) + 1;
        // all prefill gets done by thread 0, the main thread; worker threads start numbering at 1
        map.ExecuteOps(ops, 0);//, toR); 
    }

    // std::cout << "after init " << std::endl;

    //Create joinable threads
    for (unsigned i = 0; i < numThread; i++) 
    {
        thread[i] = std::thread(MapWorkThread<MapAdaptor<T> >, numThread, i + 1, testSize, tranSize, keyRange, insertion, deletion, update, std::ref(barrier), std::ref(map));
    }

    barrier.Wait();

    {
        ScopedTimer timer(true);

        //Wait for the threads to finish
        for (unsigned i = 0; i < thread.size(); i++) 
        {
            thread[i].join();
        }
    }

    map.Uninit();
}


int main(int argc, const char *argv[])
{
    uint32_t setType = 6;
    uint32_t numThread = 96;

    uint32_t testSize = 100000;
    uint32_t tranSize = 1;
    uint32_t keyRange = 100000;
    uint32_t insertion = 50;
    uint32_t deletion = 50;
    uint32_t update = 0;

    if(argc > 1) setType = atoi(argv[1]);
    if(argc > 2) numThread = atoi(argv[2]);
    if(argc > 3) testSize = atoi(argv[3]);
    if(argc > 4) tranSize = atoi(argv[4]);
    if(argc > 5) keyRange = atoi(argv[5]);
    if(argc > 6) insertion = atoi(argv[6]);
    if(argc > 7) deletion = atoi(argv[7]);
    if(argc > 8) update = atoi(argv[8]);

    assert(setType < 8);
    assert(keyRange < 0xffffffff);

    const char* setName[] = 
{         "RomulusTMLinkedListSet",
        "RomulusTMSkipListSet",
        "RomulusTMMDListSet",
        "RomulusTMHashMap",
        // "RomulusTMLinkedListSet",
        // "RomulusTMSkipListSet",
        // "RomulusTMMDListSet",
        // "RomulusTMHashMap",
        // "TransSkip" ,
        // "BoostingSkip",
        // "OSTMSkip"
    };

    printf("Start testing %s with %d threads %d iterations %d operations %d unique keys %d%% insert %d%% delete.\n", setName[setType], numThread, testSize, tranSize, keyRange, insertion, (insertion + deletion) >= 100 ? 100 - insertion : deletion);

    switch(setType)
    {
    
    case 0:
        { SetAdaptor<TMLinkedListSet<int>> set(testSize, numThread + 1, tranSize, keyRange); Tester(numThread, testSize, tranSize, keyRange, insertion, deletion, set); }
        break;
    case 1:
        { SetAdaptor<TMSkipList<int>> set(testSize, numThread + 1, tranSize, keyRange); Tester(numThread, testSize, tranSize, keyRange, insertion, deletion, set); }
        break;
    case 2:
        { SetAdaptor<TMMDListSet<int>> set(testSize, numThread + 1, tranSize, keyRange); Tester(numThread, testSize, tranSize, keyRange, insertion, deletion, set); }
        break;                
    case 3:
        { MapAdaptor<TMHashMap<int,int>> map(testSize, numThread + 1, tranSize, keyRange); MapTester(numThread, testSize, tranSize, keyRange, insertion, deletion, update, map); }
        break;                  
    // case 4:
    //     { SetAdaptor<BoostingSkip> set; Tester(numThread, testSize, tranSize, keyRange, insertion, deletion, set); }
    //     break;
    // case 5:
    //     { SetAdaptor<stm_skip> set; Tester(numThread, testSize, tranSize, keyRange, insertion, deletion, set); }
    //     break;
    default:
        break;
    }

    return 0;
}
