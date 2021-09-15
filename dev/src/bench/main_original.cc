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
#include <thread>
#include <mutex>
#include <boost/random.hpp>
#include <sched.h>
#include "../common/timehelper.h"
#include "../common/threadbarrier.h"

#include <unistd.h>


#include "setadaptor.h"




template<typename T>
void WorkThread(uint32_t numThread, int threadId, uint32_t testSize, uint32_t tranSize, uint32_t keyRange, uint32_t insertion, uint32_t deletion, ThreadBarrier& barrier,  T& set, bool recovery = false)
{
    //set affinity for each thread
    cpu_set_t cpu = {{0}};
    CPU_SET(threadId, &cpu);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpu);
    if(!recovery) {

READ_ONLY_OPT_CODE
(
        bool isReadOnly = true;
)
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
                if(op_dist <= insertion) {
                    ops[t].type = INSERT;
READ_ONLY_OPT_CODE
(                    
                    isReadOnly = false;
)
                }else if(op_dist <= insertion + deletion) {
                    ops[t].type = DELETE;
READ_ONLY_OPT_CODE
(                    
                    isReadOnly = false;
)
                }else {
                    ops[t].type = FIND;
                }
                // ops[t].type = op_dist <= insertion ? INSERT : op_dist <= insertion + deletion ? DELETE : FIND;
                ops[t].key  = randomDistKey(randomGenKey);
            }
READ_ONLY_OPT_CODE
(
            set.ExecuteOps(ops, isReadOnly);    
)

NO_READ_ONLY_OPT_CODE
(
            set.ExecuteOps(ops);    
)
            // if(i == testSize - 1)
            //     printf("%d: %d\n\r", threadId, i);    
        }

        // set.Uninit();
    }else {
        set.Init();

        barrier.Wait();
        // set.validate_and_recovery(threadCount, m_descAllocator);
    }

}

template<typename T>
void MapWorkThread(uint32_t numThread, int threadId, uint32_t testSize, uint32_t tranSize, uint32_t keyRange, uint32_t insertion, uint32_t deletion, uint32_t update, ThreadBarrier& barrier,  T& map)
{
    //set affinity for each thread
    cpu_set_t cpu = {{0}};
    CPU_SET(threadId, &cpu);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpu);
READ_ONLY_OPT_CODE
(
        bool isReadOnly = true;
)
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
READ_ONLY_OPT_CODE
(                    
                    isReadOnly = false;
)                
            }
            else if(op_dist <= insertion + deletion)
            {
                ops[t].type = MAP_DELETE;
                ops[t].value = 0;
READ_ONLY_OPT_CODE
(                    
                    isReadOnly = false;
)                
            }
            else if(op_dist <= insertion + deletion + update)
            {
                ops[t].type = MAP_UPDATE;
                ops[t].value = randomDistKey(randomGenKey) + 1;
READ_ONLY_OPT_CODE
(                    
                    isReadOnly = false;
)                
            }
            else
            {
                ops[t].type = MAP_FIND;
                ops[t].value = 0;
            }

            ops[t].key  = randomDistKey(randomGenKey) + 1;
        }

READ_ONLY_OPT_CODE
(
            map.ExecuteOps(ops, threadId, isReadOnly);//, toR);
)

NO_READ_ONLY_OPT_CODE
(
    map.ExecuteOps(ops, threadId);//, toR);
)
        //std::vector<VALUE> toR;
        
    }

    // map.Uninit();
}


template<typename T>
void TATPWorkThread(uint32_t numThread, int threadId, uint32_t testSize, uint32_t tranSize, uint32_t keyRange, uint32_t duration, bool& done, ThreadBarrier& barrier,  T& map)
{
    //set affinity for each thread
    if(numThread <= 16) {
        cpu_set_t cpu = {{0}};
        CPU_SET(threadId, &cpu);
        sched_setaffinity(0, sizeof(cpu_set_t), &cpu);
    }



    unsigned short seed[3];
    unsigned int s = rand();
    seed[0] = (unsigned short)rand_r(&s);
    seed[1] = (unsigned short)rand_r(&s);
    seed[2] = (unsigned short)rand_r(&s);


    // double startTime = Time::GetWallTime();

    // boost::mt19937 randomGenKey;

    // randomGenKey.seed(startTime + threadId);
    
    // boost::uniform_int<uint32_t> randomDistKey(1, keyRange);
    
    
    map.Init();

    barrier.Wait();
    
    MapOpArray ops(tranSize);
    ops[0].type = MAP_UPDATE;
    while(!done)
    {
        // for(uint32_t t = 0; t < tranSize; ++t)
        // {
            ops[0].key  = (uint32_t)(erand48(seed)*keyRange);//randomDistKey(randomGenKey) + 1;        
            ops[0].value = (uint32_t)(erand48(seed)*keyRange);//randomDistKey(randomGenKey) + 1;            
        // }
        map.ExecuteOpsTATP(ops, threadId);//, toR);
    }

    // map.Uninit();
}


template<typename T>
void Tester(uint32_t numThread, uint32_t testSize, uint32_t tranSize, uint32_t keyRange, uint32_t insertion, uint32_t deletion,  SetAdaptor<T>& set, bool recovery = false)
{
    std::vector<std::thread> thread(numThread);
    ThreadBarrier barrier(numThread + 1);
    if(!recovery) {


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
            thread[i] = std::thread(WorkThread<SetAdaptor<T> >, numThread, i + 1, testSize, tranSize, keyRange, insertion, deletion, std::ref(barrier), std::ref(set), recovery);
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
        // printf("all threads have joined!\n\r");

        set.Uninit();

    }else {
        // set.Init();
        //Create joinable threads
        for (unsigned i = 0; i < numThread; i++) 
        {
            thread[i] = std::thread(WorkThread<SetAdaptor<T> >, numThread, i + 1, testSize, tranSize, keyRange, insertion, deletion, std::ref(barrier), std::ref(set), recovery);
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
        // set.validate_and_recovery(numThread, m_descAllocator)

        set.Uninit();        
    }

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

template<typename T>
void TATPTester(uint32_t numThread, uint32_t testSize, uint32_t tranSize, uint32_t keyRange, uint32_t duration, MapAdaptor<T>& map)
{
    std::vector<std::thread> thread(numThread);
    ThreadBarrier barrier(numThread + 1);

    double startTime = Time::GetWallTime();
    boost::mt19937 randomGen;
    randomGen.seed(startTime - 10);
    boost::uniform_int<uint32_t> randomDist(1, keyRange);
    bool done = false;
    map.Init();

    MapOpArray ops(1);

    for(unsigned int i = 0; i < keyRange; ++i)
    {
        //std::vector<VALUE> toR;
        ops[0].type = INSERT;
        ops[0].key  = i + 1;
        ops[0].value = randomDist(randomGen) + 1;
        // all prefill gets done by thread 0, the main thread; worker threads start numbering at 1
        map.ExecuteOps(ops, 0);//, toR); 
    }

    //Create joinable threads
    for (unsigned i = 0; i < numThread; i++) 
    {
        // TATPWorkThread(uint32_t numThread, int threadId, uint32_t testSize, uint32_t tranSize, uint32_t keyRange, uint32_t duration, bool& done, ThreadBarrier& barrier,  T& map)
        thread[i] = std::thread(TATPWorkThread<MapAdaptor<T> >, numThread, i + 1, testSize, tranSize, keyRange, duration, std::ref(done), std::ref(barrier), std::ref(map));
    }

    barrier.Wait();

    {
        ScopedTimer timer(true);
        sleep(duration);
        done = true;

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
    uint32_t setType = 2;
    uint32_t numThread = 96;

    uint32_t testSize = 1000000;
    uint32_t tranSize = 1;
    uint32_t keyRange = 1000000;
    uint32_t insertion = 50;
    uint32_t deletion = 0;
    uint32_t update = 0;

    uint32_t duration = 5;

    bool recovery = false;


    if(argc > 1) setType = atoi(argv[1]);
    if(argc > 2) numThread = atoi(argv[2]);
    if(argc > 3) testSize = atoi(argv[3]);
    if(argc > 4) tranSize = atoi(argv[4]);
    if(argc > 5) keyRange = atoi(argv[5]);
    if(argc > 6) insertion = atoi(argv[6]);
    if(argc > 7) deletion = atoi(argv[7]);
    if(argc > 8) update = atoi(argv[8]);

    assert(setType < 7);
    assert(keyRange < 0xffffffff);

    const char* setName[] = 
    {   "TransList" , 
        "TransSkip",
        "TransMDList",
        "TransMap", 
        "TATPUpdateLocationBenchmark"
        // "BoostingList",
        // "TransSkip" ,
        // "BoostingSkip",
        // "OSTMSkip"
    };

    printf("Start testing %s with %d threads %d iterations %d operations %d unique keys %d%% insert %d%% delete.\n", setName[setType], numThread, testSize, tranSize, keyRange, insertion, (insertion + deletion) >= 100 ? 100 - insertion : deletion);

    // if(recovery) {
    //     printf("recovery mode\n\r");
    //     SetAdaptor<TransList> set(testSize, numThread + 1, tranSize, keyRange, recovery); 
    //     // Tester(numThread, testSize, tranSize, keyRange, insertion, deletion, set);
    //     exit(0);
        
    // }

    switch(setType)
    {

    case 0:
        { SetAdaptor<TransList> set(testSize, numThread + 1, tranSize, keyRange, recovery); Tester(numThread, testSize, tranSize, keyRange, insertion, deletion, set, recovery); }
        break;
    // case 1:
    //     { SetAdaptor<RSTMList> set; Tester(numThread, testSize, tranSize, keyRange, insertion, deletion, set); }
    //     break;
    // case 2:
    //     { SetAdaptor<BoostingList> set; Tester(numThread, testSize, tranSize, keyRange, insertion, deletion, set); }
    //     break;
    case 1:
        { SetAdaptor<trans_skip> set(testSize, numThread + 1, tranSize, keyRange); Tester(numThread, testSize, tranSize, keyRange, insertion, deletion, set); }
        break;
    case 2:
        { SetAdaptor<TxMdList> set(testSize, numThread + 1, tranSize, keyRange); Tester(numThread, testSize, tranSize, keyRange, insertion, deletion, set); }
        break;
    case 3:
        { MapAdaptor<TransMap> map(testSize, numThread + 1, tranSize, keyRange); MapTester(numThread, testSize, tranSize, keyRange, insertion, deletion, update, map); }
        break; 
    case 4:
        { MapAdaptor<TransMap> map(testSize, numThread + 1, tranSize, keyRange); TATPTester(numThread, testSize, tranSize, keyRange, duration, map); }
        break;         
    
    // case 4:
    //     { SetAdaptor<TMLinkedListSet<int>> set(testSize, numThread + 1, tranSize, keyRange); Tester(numThread, testSize, tranSize, keyRange, insertion, deletion, set); }
    //     break;         
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
