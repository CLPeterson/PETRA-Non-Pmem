#ifndef _BENCHMARK_SPEEDUP_H_
#define _BENCHMARK_SPEEDUP_H_

#include <atomic>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

#include "../commonromulus/tm.h"
#include "TMHashMapFixedSize.hpp"


using namespace std;
using namespace chrono;

template<size_t n>
struct UserData  {
    TM_TYPE<uint64_t> extra[n];
    UserData(){}
    UserData(int seq){
        extra[0] = seq;
    }
    UserData(const UserData &other){
        extra[0] = other.extra[0];
    }

    bool operator < (const UserData& other) const {
        return extra[0].pload() < other.extra[0].pload();
    }
    bool operator == (const UserData& other) const {
        return extra[0].pload() == other.extra[0].pload();
    }
};


namespace std {
    template <>
    struct hash<UserData<1>> {
        std::size_t operator()(const UserData<1>& k) const {
            using std::size_t;
            using std::hash;
            return (hash<long long>()(k.extra[0].pload()));  // This hash has no collisions, which is irealistic
        }
    };

    template <>
    struct hash<UserData<8>> {
        std::size_t operator()(const UserData<8>& k) const {
            using std::size_t;
            using std::hash;
            return (hash<long long>()(k.extra[0].pload()));  // This hash has no collisions, which is irealistic
        }
    };

    template <>
    struct hash<UserData<32>> {
        std::size_t operator()(const UserData<32>& k) const {
            using std::size_t;
            using std::hash;
            return (hash<long long>()(k.extra[0].pload()));  // This hash has no collisions, which is irealistic
        }
    };

    template <>
    struct hash<UserData<128>> {
        std::size_t operator()(const UserData<128>& k) const {
            using std::size_t;
            using std::hash;
            return (hash<long long>()(k.extra[0].pload()));  // This hash has no collisions, which is irealistic
        }
    };

    template <>
    struct hash<UserData<256>> {
        std::size_t operator()(const UserData<256>& k) const {
            using std::size_t;
            using std::hash;
            return (hash<long long>()(k.extra[0].pload()));  // This hash has no collisions, which is irealistic
        }
    };

    template <>
    struct hash<UserData<512>> {
        std::size_t operator()(const UserData<512>& k) const {
            using std::size_t;
            using std::hash;
            return (hash<long long>()(k.extra[0].pload()));  // This hash has no collisions, which is irealistic
        }
    };
}


/**
 * This is a micro-benchmark of sets, used in the CX paper
 */
class BenchmarkSpeedup {

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
    BenchmarkSpeedup(int numThreads) {
        this->numThreads = numThreads;
    }


    /**
     * When doing "updates" we execute a random removal and if the removal is successful we do an add() of the
     * same item immediately after. This keeps the size of the data structure equal to the original size (minus
     * MAX_THREADS items at most) which gives more deterministic results.
     */
    template<typename K,typename V,typename S>
    long long benchmark(const int updateRatio, const seconds testLengthSeconds, const int numRuns, const int numElements, const bool dedicated=false) {
        long long ops[numThreads][numRuns];
        long long lengthSec[numRuns];
        atomic<bool> quit = { false };
        atomic<bool> startFlag = { false };
        S* set = nullptr;

        // Create all the keys in the concurrent set
        K** udarray = new K*[numElements];
        for (int i = 0; i < numElements; i++) udarray[i] = new K(i);

        // Can either be a Reader or a Writer
        auto rw_lambda = [this,&quit,&startFlag,&set,&udarray,&numElements](const int updateRatio, long long *ops, const int tid) {
            V value{};
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
                        set->add(*udarray[ix],value);
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
            V value{};
            set->addAll(udarray, value, numElements);
            if (irun == 0) std::cout << "##### " << set->className() << " #####  \n";
            thread rwThreads[numThreads];
            if (dedicated) {
                rwThreads[0] = thread(rw_lambda, 1000, &ops[0][irun], 0);
                rwThreads[1] = thread(rw_lambda, 1000, &ops[1][irun], 1);
                for (int tid = 2; tid < numThreads; tid++) rwThreads[tid] = thread(rw_lambda, updateRatio, &ops[tid][irun], tid);
            } else {
                for (int tid = 0; tid < numThreads; tid++) rwThreads[tid] = thread(rw_lambda, updateRatio, &ops[tid][irun], tid);
            }
            this_thread::sleep_for(100ms);
            auto startBeats = steady_clock::now();
            startFlag.store(true);
            // Sleep for testLengthSeconds seconds
            this_thread::sleep_for(testLengthSeconds);
            quit.store(true);
            auto stopBeats = steady_clock::now();
            for (int tid = 0; tid < numThreads; tid++) rwThreads[tid].join();
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
            for (int tid = 0; tid < numThreads; tid++) {
                agg += ops[tid][irun]*1000000000LL/lengthSec[irun];
            }
        }

        for (int i = 0; i < numElements; i++) delete udarray[i];
        delete[] udarray;

        // Accounting
        vector<long long> agg(numRuns);
        for (int irun = 0; irun < numRuns; irun++) {
            for (int tid = 0; tid < numThreads; tid++) {
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
        vector<int> threadList = { 1, 2, 4, 16, 30, 64 };  // For Cervino
        //vector<int> threadList = { 1,2,4,8};                            // For the laptop
        vector<int> ratioList = { 1000/*, 100, 0 */ }; // Permil ratio: 100%, 10%, 0%
        vector<long long> elemsList = { 100 }; // Number of keys in the set
        const int numRuns = 1;                        // 5 runs for the paper
        const seconds testLength = 20s;                // 20s for the paper
        const bool dedicated = false;                 // If "true" make sure threadList starts at 2 or higher. It is used for the read-while-writing scenarios
        const int EMAX_STRUCT = 4;
        const int EMAX_VALUE_SIZE = 6;

        long long ops[EMAX_STRUCT][elemsList.size()][ratioList.size()][threadList.size()][EMAX_VALUE_SIZE];

        double totalHours = (double)EMAX_STRUCT*elemsList.size()*ratioList.size()*threadList.size()*testLength.count()*numRuns/(60.*60.);
        std::cout << "This benchmark is going to take about " << totalHours << " hours to complete\n";
        if (dedicated) std::cout << "Dedicated Enabled: 2 extra threads will start. They are dedicated writers and the remaining threads are mixed ratio\n";

        for (unsigned ielem = 0; ielem < elemsList.size(); ielem++) {
            auto numElements = elemsList[ielem];
            for (unsigned iratio = 0; iratio < ratioList.size(); iratio++) {
                auto ratio = ratioList[iratio];
                for (unsigned ithread = 0; ithread < threadList.size(); ithread++) {
                    // Initialize operation counters
                    for (int itype = 0; itype < EMAX_STRUCT; itype++) {
                        for (int ivs = 0; ivs < EMAX_VALUE_SIZE; ivs++) {
                            ops[itype][ielem][iratio][ithread][ivs] = 0;
                        }
                    }
                    auto nThreads = threadList[ithread];
                    if (dedicated) nThreads += 2;  // Add two extra threads to make mutative operations
                    BenchmarkSpeedup bench(nThreads);
                    std::cout << "\n----- HashMapFixed Speedup Benchmark   keys=" << numElements << "   ratio=" << ratio/10. << "%   threads=" << nThreads << "   runs=" << numRuns << "   length=" << testLength.count() << "s -----\n";
                    std::cout << "Value = 8 bytes\n";
                    ops[3][ielem][iratio][ithread][0] = bench.benchmark<UserData<1>,UserData<1>,TMHashMapFixedSize<UserData<1>,UserData<1>>>(ratio, testLength, numRuns, numElements, dedicated);
                    std::cout << "\nValue = 64 bytes\n";
                    ops[3][ielem][iratio][ithread][1] = bench.benchmark<UserData<1>,UserData<8>,TMHashMapFixedSize<UserData<1>,UserData<8>>>(ratio, testLength, numRuns, numElements, dedicated);
                    std::cout << "\nValue = 256 bytes\n";
                    ops[3][ielem][iratio][ithread][2] = bench.benchmark<UserData<1>,UserData<32>,TMHashMapFixedSize<UserData<1>,UserData<32>>>(ratio, testLength, numRuns, numElements, dedicated);
                    std::cout << "\nValue = 1024 bytes\n";
                    ops[3][ielem][iratio][ithread][3] = bench.benchmark<UserData<1>,UserData<128>,TMHashMapFixedSize<UserData<1>,UserData<128>>>(ratio, testLength, numRuns, numElements, dedicated);
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
                        for (int ivs = 0; ivs < EMAX_VALUE_SIZE; ivs++) {
                            std::cout << ops[il][ielem][iratio][ithread][ivs] << ", ";
                        }
                    }
                    std::cout << "\n";
                }
            }
        }
    }


    // A single iteration to be executed form the command line directly.
    // This is needed because Mnemosyne crashes all the time, so we have to run one iteration at a time, and clear the /dev/shm/psegments in between.
    static void singleTest(int value, int iclass, long nThreads, long ratio, long numElements, seconds testLength=20s, int numRuns=1, bool dedicated=false) {
        BenchmarkSpeedup bench(nThreads);
        std::cout << "\n----- HashMapFixed Speedup Benchmark   keys=" << numElements << "   ratio=" << ratio/10. << "%   threads=" << nThreads << "   runs=" << numRuns << "   length=" << testLength.count() << "s -----\n";
        if (iclass != 3) return;
        if (value == 8) {
            std::cout << "Value = 8 bytes\n";
            bench.benchmark<UserData<1>,UserData<1>,TMHashMapFixedSize<UserData<1>,UserData<1>>>(ratio, testLength, numRuns, numElements, dedicated);
        } else if (value == 64) {
            std::cout << "\nValue = 64 bytes\n";
            bench.benchmark<UserData<1>,UserData<8>,TMHashMapFixedSize<UserData<1>,UserData<8>>>(ratio, testLength, numRuns, numElements, dedicated);
        } else if (value == 256) {
            std::cout << "\nValue = 256 bytes\n";
            bench.benchmark<UserData<1>,UserData<32>,TMHashMapFixedSize<UserData<1>,UserData<32>>>(ratio, testLength, numRuns, numElements, dedicated);
        } else if (value == 1024) {
            std::cout << "\nValue = 1024 bytes\n";
            bench.benchmark<UserData<1>,UserData<128>,TMHashMapFixedSize<UserData<1>,UserData<128>>>(ratio, testLength, numRuns, numElements, dedicated);
        }
    }

};

#endif
