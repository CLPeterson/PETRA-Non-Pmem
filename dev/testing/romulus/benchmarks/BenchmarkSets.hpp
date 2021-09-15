#ifndef _BENCHMARK_SETS_H_
#define _BENCHMARK_SETS_H_

#include <atomic>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

#include "../common/tm.h"
#include "../datastructures/TMRedBlackBST.hpp"
#include "../datastructures/TMHashMap.hpp"
#include "../datastructures/TMHashMapFixedSize.hpp"
#include "../datastructures/TMLinkedListSet.hpp"


using namespace std;
using namespace chrono;

struct UserData  {
    TM_TYPE<long long> seq;
    TM_TYPE<int> tid;
    UserData(long long lseq, int ltid=0) {
        this->seq = lseq;
        this->tid = ltid;
    }
    UserData() {
        this->seq = -2;
        this->tid = -2;
    }
    UserData(const UserData &other) : seq(other.seq), tid(other.tid) { }

    bool operator < (const UserData& other) const {
        return seq.pload() < other.seq.pload();
    }
    bool operator == (const UserData& other) const {
        return seq.pload() == other.seq.pload() && tid.pload() == other.tid.pload();
    }
};

namespace std {
    template <>
    struct hash<UserData> {
        std::size_t operator()(const UserData& k) const {
            using std::size_t;
            using std::hash;
            return (hash<long long>()(k.seq.pload()));  // This hash has no collisions, which is irealistic
            /*
            long long x = k.seq;
            x ^= x >> 12; // a
            x ^= x << 25; // b
            x ^= x >> 27; // c
            return hash<long long>()(x * 2685821657736338717LL);
            */
        }
    };
}


/**
 * This is a micro-benchmark of sets, used in the CX paper
 */
template<typename K>
class BenchmarkSets {

private:
    struct Result {
        nanoseconds nsEnq = 0ns;
        nanoseconds nsDeq = 0ns;
        long long numEnq = 0;
        long long numDeq = 0;
        long long totOpsSec = 0;

        Result() { }

        Result(const Result &other) {
            nsEnq = other.nsEnq;
            nsDeq = other.nsDeq;
            numEnq = other.numEnq;
            numDeq = other.numDeq;
            totOpsSec = other.totOpsSec;
        }

        bool operator < (const Result& other) const {
            return totOpsSec < other.totOpsSec;
        }
    };

    static const long long NSEC_IN_SEC = 1000000000LL;

    int numThreads;

public:
    BenchmarkSets(int numThreads) {
        this->numThreads = numThreads;
    }


    /**
     * When doing "updates" we execute a random removal and if the removal is successful we do an add() of the
     * same item immediately after. This keeps the size of the data structure equal to the original size (minus
     * MAX_THREADS items at most) which gives more deterministic results.
     */
    template<typename S>
    long long benchmark(const int updateRatio, const seconds testLengthSeconds, const int numRuns, const int numElements, const bool dedicated=false) {
    	int num_threads = numThreads;
    	if(dedicated) num_threads = numThreads+2;
        long long ops[num_threads][numRuns];
        long long lengthSec[numRuns];
        atomic<bool> quit = { false };
        atomic<bool> startFlag = { false };
        S* set = nullptr;

        // Create all the keys in the concurrent set
        K** udarray = new K*[numElements];
        for (int i = 0; i < numElements; i++) udarray[i] = new K(i);

        // Can either be a Reader or a Writer
        auto rw_lambda = [this,&quit,&startFlag,&set,&udarray,&numElements](const int updateRatio, long long *ops, const int tid) {
        	uint64_t accum = 0;
            long long numOps = 0;
            while (!startFlag.load()) ; // spin
            uint64_t seed = tid+1234567890123456781ULL;
            while (!quit.load()) {
                seed = randomLong(seed);
                int update = seed%1000;
                seed = randomLong(seed);
                auto ix = (unsigned int)(seed%numElements);
                if (update < updateRatio) {
                    // I'm a Writer
                    if (set->remove(*udarray[ix])) {
                    	numOps++;
                    	set->add(*udarray[ix]);
                    }
                    numOps++;
                } else {
                	// I'm a Reader
                    set->contains(*udarray[ix]);
                    seed = randomLong(seed);
                    ix = (unsigned int)(seed%numElements);
                    set->contains(*udarray[ix]);
                    numOps+=2;
                }

            }
            *ops = numOps;
        };

        for (int irun = 0; irun < numRuns; irun++) {
            TM_WRITE_TRANSACTION([&] () {
                set = TM_ALLOC<S>();
            });
            // Add all the items to the list
            set->addAll(udarray, numElements);
            if (irun == 0) std::cout << "##### " << set->className() << " #####  \n";
            thread rwThreads[num_threads];

            if (dedicated) {
                rwThreads[0] = thread(rw_lambda, 1000, &ops[0][irun], 0);
                rwThreads[1] = thread(rw_lambda, 1000, &ops[1][irun], 1);
                for (int tid = 2; tid < num_threads; tid++) rwThreads[tid] = thread(rw_lambda, updateRatio, &ops[tid][irun], tid);
            } else {
                for (int tid = 0; tid < num_threads; tid++) rwThreads[tid] = thread(rw_lambda, updateRatio, &ops[tid][irun], tid);
            }
            this_thread::sleep_for(100ms);
            auto startBeats = steady_clock::now();
            startFlag.store(true);
            // Sleep for testLengthSeconds seconds
            this_thread::sleep_for(testLengthSeconds);
            quit.store(true);
            auto stopBeats = steady_clock::now();
            for (int tid = 0; tid < num_threads; tid++) {
            	rwThreads[tid].join();
            }
            lengthSec[irun] = (stopBeats-startBeats).count();
            if (dedicated) {
                // We don't account for the write-only operations but we aggregate the values from the two threads and display them
                std::cout << "Mutative transactions per second = " << (ops[0][irun] + ops[1][irun])*1000000000LL/lengthSec[irun] << "\n";
                ops[0][irun] = 0;
                ops[1][irun] = 0;
            }
            quit.store(false);
            startFlag.store(false);
            // Measure the time the destructor takes to complete and if it's more than 1 second, print it out
            auto startDel = steady_clock::now();
            TM_WRITE_TRANSACTION([&] () {
                TM_FREE(set);
            });
            auto stopDel = steady_clock::now();
            if ((startDel-stopDel).count() > NSEC_IN_SEC) {
                std::cout << "Destructor took " << (startDel-stopDel).count()/NSEC_IN_SEC << " seconds\n";
            }

            // Compute ops at the end of each run
            long long agg = 0;
            for (int tid = 0; tid < num_threads; tid++) {
                agg += ops[tid][irun]*1000000000LL/lengthSec[irun];
            }
        }

        for (int i = 0; i < numElements; i++) delete udarray[i];
        delete[] udarray;

        // Accounting
        vector<long long> agg(numRuns);
        for (int irun = 0; irun < numRuns; irun++) {
            for (int tid = 0; tid < num_threads; tid++) {
                agg[irun] += ops[tid][irun]*1000000000LL/lengthSec[irun];
            }
        }

        // Compute the median. numRuns must be an odd number
        sort(agg.begin(),agg.end());
        auto maxops = agg[numRuns-1];
        auto minops = agg[0];
        auto medianops = agg[numRuns/2];
        auto delta = (long)(100.*(maxops-minops) / ((double)medianops));
        // Printed value is the median of the number of ops per second that all threads were able to accomplish (on average)
        std::cout << "Ops/sec = " << medianops << "      delta = " << delta << "%   min = " << minops << "   max = " << maxops << "\n";
        return medianops;
    }


    /**
     * An imprecise but fast random number generator
     */
    uint64_t randomLong(uint64_t x) {
        x ^= x >> 12; // a
        x ^= x << 25; // b
        x ^= x >> 27; // c
        return x * 2685821657736338717LL;
    }


public:

    static void allThroughputTests() {
        //vector<int> threadList = { 1, 2, 4, 8, 16, 24, 30, 32, 40, 48, 56, 64 };  // For Cervino
        vector<int> threadList = { 1, 2, 4, 8, 16, 24, 48, 96 };                            // For the laptop
        vector<int> ratioList = { 1000, /* 500, 100, 10, 1, */0 };//{500, 330, 150/*100, 10, 1, */}; // Permil ratio: 100%, 50%, 10%, 1%, 0.1%, 0%
        vector<long long> elemsList = {1000};// {  100000/*, 100000, 1000000*/ }; // Number of keys in the set
        const int numRuns = 3;                        // 5 runs for the paper
        const seconds testLength = 20s;                // 20s for the paper
        const bool dedicated = false;                 // If "true" make sure threadList starts at 2 or higher. It is used for the read-while-writing scenarios
        const int EMAX_STRUCT = 1;

        long long ops[EMAX_STRUCT][elemsList.size()][ratioList.size()][threadList.size()];

#ifdef PMDK_PTM
        std::cout << "If you're using PMDK on DRAM, don't forget to set export PMEM_IS_PMEM_FORCE=1\n";
#endif
        double totalHours = (double)EMAX_STRUCT*elemsList.size()*ratioList.size()*threadList.size()*testLength.count()*numRuns/(60.*60.);
        std::cout << "This benchmark is going to take about " << totalHours << " hours to complete\n";
        if (dedicated) std::cout << "Dedicated Enabled: 2 extra threads will start. They are dedicated writers and the remaining threads are mixed ratio\n";

        for (unsigned ielem = 0; ielem < elemsList.size(); ielem++) {
            auto numElements = elemsList[ielem];
            for (unsigned iratio = 0; iratio < ratioList.size(); iratio++) {
                auto ratio = ratioList[iratio];
                for (unsigned ithread = 0; ithread < threadList.size(); ithread++) {
                    // Initialize operation counters
                    for (int itype = 0; itype < EMAX_STRUCT; itype++) ops[itype][ielem][iratio][ithread] = 0;
                    auto nThreads = threadList[ithread];
                    //if (dedicated) nThreads += 2;  // Add two extra threads to make mutative operations
                    BenchmarkSets bench(nThreads);
                    std::cout << "\n----- Sets Benchmark   numElements=" << numElements << "   ratio=" << ratio/10. << "%   threads=" << nThreads << "   runs=" << numRuns << "   length=" << testLength.count() << "s -----\n";
                    ops[0][ielem][iratio][ithread] = bench.benchmark<TMLinkedListSet<K>>(ratio, testLength, numRuns, numElements, dedicated);
                    ///ops[1][ielem][iratio][ithread] = bench.benchmark<TMRedBlackBST<K,K>>(ratio, testLength, numRuns, numElements, dedicated);
                    ///ops[2][ielem][iratio][ithread] = bench.benchmark<TMHashMap<K,K>>(ratio, testLength, numRuns, numElements, dedicated);
                    ///ops[3][ielem][iratio][ithread] = bench.benchmark<TMHashMapFixedSize<K,K>>(ratio, testLength, numRuns, numElements, dedicated);
                }
            }
        }

        // Show results in csv format
        std::cout << "\n\nResults in ops per second   numRuns=" << numRuns << ",  length=" << testLength.count() << "s \n";
        for (unsigned ielem = 0; ielem < elemsList.size(); ielem++) {
            auto numElements = elemsList[ielem];
            std::cout << "\nNumber of elements: " << numElements << "\n";
            for (unsigned iratio = 0; iratio < ratioList.size(); iratio++) {
                auto ratio = ratioList[iratio]/10.;
                std::cout << "Ratio " << ratio << "%\n";
                std::cout << "Threads\n";
                for (unsigned ithread = 0; ithread < threadList.size(); ithread++) {
                    auto nThreads = threadList[ithread];
                    if (dedicated) nThreads += 2;  // Add two extra threads to make mutative operations
                    std::cout << nThreads << ", ";
                    for (int il = 0; il < EMAX_STRUCT; il++) {
                        std::cout << ops[il][ielem][iratio][ithread] << ", ";
                    }
                    std::cout << "\n";
                }
            }
        }
    }


    // A single iteration to be executed form the command line directly.
    // This is needed because Mnemosyne crashes all the time, so we have to run one iteration at a time, and clear the /dev/shm/psegments in between.
    static void singleTest(int iclass, long nThreads, long ratio, long numElements, seconds testLength=20s, int numRuns=1, bool dedicated=false) {
        BenchmarkSets bench(nThreads);
        std::cout << "\n----- Single Sets Benchmark   numElements=" << numElements << "   ratio=" << ratio/10. << "%   threads=" << nThreads << "   runs=" << numRuns << "   length=" << testLength.count() << "s -----\n";
        if (iclass == 0) bench.benchmark<TMLinkedListSet<K>>(ratio, testLength, numRuns, numElements, dedicated);
        if (iclass == 1) bench.benchmark<TMRedBlackBST<K,K>>(ratio, testLength, numRuns, numElements, dedicated);
        if (iclass == 2) bench.benchmark<TMHashMap<K,K>>(ratio, testLength, numRuns, numElements, dedicated);
        //if (iclass == 3) bench.benchmark<TMHashMapFixedSize<K,K>>(ratio, testLength, numRuns, numElements, dedicated);
    }

};

#endif
