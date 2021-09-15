#ifndef _BENCHMARK_PERSISTENCY_H_
#define _BENCHMARK_PERSISTENCY_H_

#include <atomic>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <typeinfo>

// Same as persist<> but specialized for each persistency engine
template<typename PE>
struct PersistentArray;

#include "common/ThreadRegistry.hpp"
#include "romulus/Romulus.hpp"
#include "romuluslog/RomulusLog.hpp"
#include "romuluslr/RomulusLR.hpp"
#include "romulusni/RomulusNI.hpp"
//#include "experimental/romulussf/RomulusSF.hpp"
#include "pmdk/PMDKTM.hpp"                // Needs pmem installed (pmdk)
// This is defined in SConscript for builds done inside the mnemosyne-gcc repository
#ifdef USE_MNEMOSYNE
#include "Mnemosyne.hpp"
#endif


// We tried 1024 but it causes SIGFPE and SIGSEGV pn pmalloc() and pfree() of Mnemosyne. With 512 seems to work... buggy
// For SPS we can do 10,000 because Mnemosyne still holds it
static const int arraySize=10000;

struct WrapperR {
    romulus::persist<int64_t> val;
    WrapperR(int64_t val): val {val} { }
};

struct WrapperRL {
    romuluslog::persist<int64_t> val;
    WrapperRL(int64_t val): val {val} { }
};

struct WrapperR_LR {
    romuluslr::persist<int64_t> val;
    WrapperR_LR(int64_t val): val {val} { }
};

template<>
struct PersistentArray<romulus::Romulus> {
    romulus::persist<WrapperR*> counters[arraySize];
    PersistentArray(){
        for (int i = 0; i < arraySize; i++){
            counters[i] = romulus::Romulus::alloc<WrapperR>(0);
        }
    }
    ~PersistentArray(){}
};

template<>
struct PersistentArray<romuluslog::RomulusLog> {
    romuluslog::persist<WrapperRL*> counters[arraySize];
    PersistentArray() {
        for (int i = 0; i < arraySize; i++){
            counters[i] = romuluslog::RomulusLog::alloc<WrapperRL>(0);
        }
    }
    ~PersistentArray(){}
};

template<>
struct PersistentArray<romuluslr::RomulusLR> {
    romuluslr::persist<WrapperR_LR*> counters[arraySize];
    PersistentArray() {
        for (int i = 0; i < arraySize; i++){
            counters[i] = romuluslr::RomulusLR::alloc<WrapperR_LR>(0);
        }
    }
    ~PersistentArray(){}
};


template<template<typename T> class persist>
struct PersistentArrayInt {
    persist<int64_t> counters[arraySize];
    PersistentArrayInt() {
        for (int i = 0; i < arraySize; i++) counters[i] = 0;
    }
};

#ifdef USE_MNEMOSYNE

struct WrapperM {
    mnemosyne::persist<int64_t> val;
    WrapperM(int64_t val): val {val} { }
};

template<>
struct PersistentArray<mnemosyne::Mnemosyne> {
    mnemosyne::persist<WrapperM*> counters[arraySize];
    PersistentArray() {
        for (int i = 0; i < arraySize; i++){
            counters[i] = mnemosyne::Mnemosyne::alloc<WrapperM>(0);
        }
    }
    ~PersistentArray() {
    }
};
#endif

using namespace std;
using namespace chrono;


/**
 * This is a micro-benchmark
 */
class BenchmarkPersistency {

private:

    // Performance benchmark constants
    static const long long kNumPairsWarmup =     1000000LL;     // Each threads does 1M iterations as warmup

    // Contants for Ping-Pong performance benchmark
    static const int kPingPongBatch = 1000;            // Each thread starts by injecting 1k items in the queue

    static const long long NSEC_IN_SEC = 1000000000LL;

    int numThreads;

    static const int PIDX_ARRAY = 0;
    static const int PIDX_QUEUE = 1;
    static const int PIDX_INT_ARRAY = 2;
    static const int PIDX_SPS = 3;

public:
    struct UserData  {
        long long seq;
        int tid;
        UserData(long long lseq, int ltid) {
            this->seq = lseq;
            this->tid = ltid;
        }
        UserData() {
            this->seq = -2;
            this->tid = -2;
        }
        UserData(const UserData &other) : seq(other.seq), tid(other.tid) { }

        bool operator < (const UserData& other) const {
            return seq < other.seq;
        }
    };

    BenchmarkPersistency(int numThreads) {
        this->numThreads = numThreads;
    }


    /*
     * An array of integers that gets randomly permutated.
     * The same as in NV Heaps, Blurred Persistence, etc
     */
    template<typename PE, template<typename T> class persist>
    long long benchmarkSPS(const seconds testLengthSeconds, const long numWordsPerTransaction, const int numRuns) {
        long long ops[numThreads][numRuns];
        long long lengthSec[numRuns];
        atomic<bool> startFlag = { false };
        atomic<bool> quit = { false };
        // Create the array of integers and initialize it, unless it is already there
        persist<uint64_t>* parray;
        PE::write_transaction([this,&parray] () {
            parray = PE::template get_object<persist<uint64_t>>(PIDX_SPS);
            if (parray == nullptr) {
                parray = (persist<uint64_t>*)PE::pmalloc(arraySize*sizeof(persist<uint64_t>));
                for (int i = 0; i < arraySize; i++) parray[i] = i;
                PE::put_object(PIDX_SPS, parray);
            } else {
                // Check that the array is consistent
                uint64_t sum1 = 0, sum2 = 0;
                for (int i = 0; i < arraySize; i++) sum1 += i;
                for (int i = 0; i < arraySize; i++) sum2 += parray[i];
                assert(sum1 == sum2);
            }
        });
        auto func = [this,&startFlag,&quit,&numWordsPerTransaction,&parray](long long *ops, const int tid) {
            uint64_t seed = tid+1234567890123456781ULL;
            // Spin until the startFlag is set
            while (!startFlag.load()) {}
            // Do transactions until the quit flag is set
            long long tcount = 0;
            while (!quit.load()) {
                PE::write_transaction([this,&seed,&parray,&numWordsPerTransaction] () {
                    for (int i = 0; i < numWordsPerTransaction; i++) {
                        seed = randomLong(seed);
                        auto ia = seed%arraySize;
                        uint64_t tmp = parray[ia];
                        seed = randomLong(seed);
                        auto ib = seed%arraySize;
                        parray[ia] = parray[ib];
                        parray[ib] = tmp;
                    }
                } );
                ++tcount;
            }
            *ops = tcount;
        };
        for (int irun = 0; irun < numRuns; irun++) {
            if (irun == 0) cout << "##### " << PE::className() << " #####  \n";
            thread enqdeqThreads[numThreads];
            for (int tid = 0; tid < numThreads; tid++) enqdeqThreads[tid] = thread(func, &ops[tid][irun], tid);
            auto startBeats = steady_clock::now();
            startFlag.store(true);
            // Sleep for 20 seconds
            this_thread::sleep_for(testLengthSeconds);
            quit.store(true);
            auto stopBeats = steady_clock::now();
            for (int tid = 0; tid < numThreads; tid++) enqdeqThreads[tid].join();
            lengthSec[irun] = (stopBeats-startBeats).count();
            startFlag.store(false);
            quit.store(false);
        }

        PE::write_transaction([&parray] () {
            PE::pfree(parray);
            PE::template put_object<persist<uint64_t>>(PIDX_SPS, nullptr);
        });
        // Accounting
        vector<long long> agg(numRuns);
        for (int irun = 0; irun < numRuns; irun++) {
        	for(int i=0;i<numThreads;i++){
        		agg[irun] += ops[i][irun]*1000000000LL/lengthSec[irun];
        	}
        }
        // Compute the median. numRuns should be an odd number
        sort(agg.begin(),agg.end());
        auto maxops = agg[numRuns-1];
        auto minops = agg[0];
        auto medianops = agg[numRuns/2];
        auto delta = (long)(100.*(maxops-minops) / ((double)medianops));
        // Printed value is the median of the number of ops per second that all threads were able to accomplish (on average)
        std::cout << "Transactions/sec = " << medianops << "     delta = " << delta << "%   min = " << minops << "   max = " << maxops << "\n";
        return medianops;
    }



    /*
     * Runs a Persistency Engine over an array of integers with (hard-coded) size of 1024
     */
    template<typename PE, typename W>
    long long arrayBenchmark(const seconds testLengthSeconds, const long numWordsPerTransaction, const int numRuns) {
        long long ops[numRuns];
        long long lengthSec[numRuns];
        atomic<bool> startFlag = { false };
        atomic<bool> quit = { false };

        PE::init();
        PE pe {};
        auto func = [this,&startFlag,&quit,&numWordsPerTransaction,&pe](long long *ops, const int tid) {
            uint64_t seed = tid+1234567890123456781ULL;
            // Create the array of integers and initialize it, unless it is already there
            PersistentArray<PE>* parray;
            pe.transaction([this,&parray,&pe] () {
                parray = pe.template get_object<PersistentArray<PE>>(PIDX_ARRAY);
                if (parray == nullptr) {
                    parray = pe.template alloc<PersistentArray<PE>>(pe);
                    pe.put_object(PIDX_ARRAY, parray);
                } else {
                    // Check that the array is consistent
                    int64_t sum = 0;
                    for (int i = 0; i < arraySize; i++) {
                        sum += parray->counters[i].pload()->val;
                    }
                    assert(sum == 0);
                }
            });
            // Spin until the startFlag is set
            while (!startFlag.load()) {}
            // Do transactions until the quit flag is set
            long long tcount = 0;
            while (!quit.load()) {
                pe.transaction([this,&seed,&parray,&numWordsPerTransaction,&pe] () {
                    for (int i = 0; i < numWordsPerTransaction/2; i++) {
                        seed = randomLong(seed);
                        W* old =parray->counters[seed%arraySize];
                        W* next =pe.template alloc<W>(old->val-1);
                        parray->counters[seed%arraySize]=next;  // Subtract from random places
                        pe.free(old);
                    }
                    for (int i = 0; i < numWordsPerTransaction/2; i++) {
                        seed = randomLong(seed);
                        W* old =parray->counters[seed%arraySize];
                        W* next =pe.template alloc<W>(old->val+1);
                        parray->counters[seed%arraySize]=next;  // Add from random places
                        pe.free(old);
                    }
                } );
                tcount++;
            }
            *ops = tcount;
            pe.transaction([&pe,&parray] () {
                for(int i=0;i<arraySize;i++){
                    pe.free(parray->counters[i].pload());
                }
                // This free() causes Mnemosyne to crash, don't know why (yet)
                pe.free(parray);
                pe.template put_object<PersistentArray<PE>>(0, nullptr);

            });
        };
        for (int irun = 0; irun < numRuns; irun++) {
            if (irun == 0) cout << "##### " << pe.className() << " #####  \n";
            thread enqdeqThreads[numThreads];
            for (int tid = 0; tid < numThreads; tid++) enqdeqThreads[tid] = thread(func, &ops[irun], tid);
            auto startBeats = steady_clock::now();
            startFlag.store(true);
            // Sleep for 20 seconds
            this_thread::sleep_for(testLengthSeconds);
            quit.store(true);
            auto stopBeats = steady_clock::now();
            for (int tid = 0; tid < numThreads; tid++) enqdeqThreads[tid].join();
            lengthSec[irun] = (stopBeats-startBeats).count();
            startFlag.store(false);
            quit.store(false);
        }
        // Accounting
        vector<long long> agg(numRuns);
        for (int irun = 0; irun < numRuns; irun++) {
            agg[irun] += ops[irun]*1000000000LL/lengthSec[irun];
        }
        // Compute the median. numRuns should be an odd number
        sort(agg.begin(),agg.end());
        auto maxops = agg[numRuns-1];
        auto minops = agg[0];
        auto medianops = agg[numRuns/2];
        auto delta = (long)(100.*(maxops-minops) / ((double)medianops));
        // Printed value is the median of the number of ops per second that all threads were able to accomplish (on average)
        std::cout << "Transactions/sec = " << medianops << "     delta = " << delta << "%   min = " << minops << "   max = " << maxops << "\n";
        return medianops;
    }


    /*
     * Runs a Persistency Engine over an array of Wrappers with (hard-coded) size of 512
     */
    template<typename PE, typename W>
    long long arrayBenchmark_rw(const seconds testLengthSeconds, const long numWordsPerTransaction, const int numRuns) {
        long long ops[numRuns];
        long long lengthSec[numRuns];
        atomic<bool> startFlag = { false };
        atomic<bool> quit = { false };

        PE::init();
        // Create the array of integers and initialize it, unless it is already there
        PersistentArray<PE>* parray;
        PE::write_transaction([this,&parray] () {
            parray = PE::template get_object<PersistentArray<PE>>(PIDX_ARRAY);
            if (parray == nullptr) {
                parray = PE::template alloc<PersistentArray<PE>>();
                PE::put_object(PIDX_ARRAY, parray);
            } else {
                // Check that the array is consistent
                int64_t sum = 0;
                for (int i = 0; i < arraySize; i++) {
                    sum += parray->counters[i].pload()->val;
                }
                assert(sum == 0);
            }
        });
        auto func = [this,&startFlag,&quit,&numWordsPerTransaction,&parray](long long *ops, const int tid) {
            uint64_t seed = tid+1234567890123456781ULL;
            // Spin until the startFlag is set
            while (!startFlag.load()) {}
            // Do transactions until the quit flag is set
            long long tcount = 0;
            while (!quit.load()) {
                PE::write_transaction([this,&seed,&parray,&numWordsPerTransaction] () {
                    for (int i = 0; i < numWordsPerTransaction/2; i++) {
                        seed = randomLong(seed);
                        W* old = parray->counters[seed%arraySize];
                        W* next = PE::template alloc<W>(old->val-1);
                        parray->counters[seed%arraySize]=next;  // Subtract from random places
                        PE::free(old);
                    }
                    for (int i = 0; i < numWordsPerTransaction/2; i++) {
                        seed = randomLong(seed);
                        W* old = parray->counters[seed%arraySize];
                        W* next = PE::template alloc<W>(old->val+1);
                        parray->counters[seed%arraySize]=next;  // Add from random places
                        PE::free(old);
                    }
                } );

                PE::read_transaction([this,&seed,&parray,&numWordsPerTransaction] () {
                    // Check that the array is consistent
                    int64_t sum = 0;
                    for (int i = 0; i < arraySize; i++) {
                        sum += parray->counters[i].pload()->val;
                    }
                    assert(sum == 0);
                } );
                tcount=tcount+2;
            }
            *ops = tcount;
        };
        for (int irun = 0; irun < numRuns; irun++) {
            if (irun == 0) cout << "##### " << PE::className() << " #####  \n";
            thread enqdeqThreads[numThreads];
            for (int tid = 0; tid < numThreads; tid++) enqdeqThreads[tid] = thread(func, &ops[irun], tid);
            auto startBeats = steady_clock::now();
            startFlag.store(true);
            // Sleep for 20 seconds
            this_thread::sleep_for(testLengthSeconds);
            quit.store(true);
            auto stopBeats = steady_clock::now();
            for (int tid = 0; tid < numThreads; tid++) enqdeqThreads[tid].join();
            lengthSec[irun] = (stopBeats-startBeats).count();
            startFlag.store(false);
            quit.store(false);
        }

        PE::write_transaction([&parray] () {
            for(int i=0;i<arraySize;i++){
                PE::free(parray->counters[i].pload());
            }
            // This free() causes Mnemosyne to crash, don't know why (yet)
            PE::free(parray);
            PE::template put_object<PersistentArray<PE>>(PIDX_ARRAY, nullptr);

        });
        // Accounting
        vector<long long> agg(numRuns);
        for (int irun = 0; irun < numRuns; irun++) {
            agg[irun] += ops[irun]*1000000000LL/lengthSec[irun];
        }
        // Compute the median. numRuns should be an odd number
        sort(agg.begin(),agg.end());
        auto maxops = agg[numRuns-1];
        auto minops = agg[0];
        auto medianops = agg[numRuns/2];
        auto delta = (long)(100.*(maxops-minops) / ((double)medianops));
        // Printed value is the median of the number of ops per second that all threads were able to accomplish (on average)
        std::cout << "Transactions/sec = " << medianops << "     delta = " << delta << "%   min = " << minops << "   max = " << maxops << "\n";
        return medianops;
    }


    /*
     * Runs a Persistency Engine over an array of integers with (hard-coded) size of 512
     */
    template<typename PE, template<typename T> class persist>
    long long integerArrayBenchmark_rw(const seconds testLengthSeconds, const long numWordsPerTransaction, const int numRuns) {
        long long ops[numRuns];
        long long lengthSec[numRuns];
        atomic<bool> startFlag = { false };
        atomic<bool> quit = { false };

        PE::init();
        // Create the array of integers and initialize it, unless it is already there
        PersistentArrayInt<persist>* parray;
        PE::write_transaction([this,&parray] () {
            parray = PE::template get_object<PersistentArrayInt<persist>>(PIDX_INT_ARRAY);
            if (parray == nullptr) {
                parray = PE::template alloc<PersistentArrayInt<persist>>();
                PE::put_object(PIDX_INT_ARRAY, parray);
            } else {
                // Check that the array is consistent
                int sum = 0;
                for (int i = 0; i < arraySize; i++) {
                    sum += parray->counters[i];
                }
                assert(sum == 0);
            }
        });
        auto func = [this,&startFlag,&quit,&numWordsPerTransaction,&parray](long long *ops, const int tid) {
            uint64_t seed = tid+1234567890123456781ULL;
            // Spin until the startFlag is set
            while (!startFlag.load()) {}
            // Do transactions until the quit flag is set
            long long tcount = 0;
            while (!quit.load()) {
                PE::write_transaction([this,&seed,&parray,&numWordsPerTransaction] () {
                    for (int i = 0; i < numWordsPerTransaction/2; i++) {
                        seed = randomLong(seed);
                        parray->counters[seed%arraySize] = parray->counters[seed%arraySize]-1;
                    }
                    for (int i = 0; i < numWordsPerTransaction/2; i++) {
                        seed = randomLong(seed);
                        parray->counters[seed%arraySize] = parray->counters[seed%arraySize]+1;
                    }
                } );
/*
                PE::read_transaction([this,&seed,&parray,&numWordsPerTransaction] () {
                    PersistentArrayInt<persist>* read_array = PE::template get_object<PersistentArrayInt<persist>>(PIDX_INT_ARRAY);
                    // Check that the array is consistent
                    int sum = 0;
                    for (int i = 0; i < arraySize; i++) {
                        sum += read_array->counters[i];
                    }
                    assert(sum == 0);
                } );
*/
                tcount=tcount+2;
            }
            *ops = tcount;
        };
        for (int irun = 0; irun < numRuns; irun++) {
            if (irun == 0) cout << "##### " << PE::className() << " #####  \n";
            thread enqdeqThreads[numThreads];
            for (int tid = 0; tid < numThreads; tid++) enqdeqThreads[tid] = thread(func, &ops[irun], tid);
            auto startBeats = steady_clock::now();
            startFlag.store(true);
            // Sleep for 20 seconds
            this_thread::sleep_for(testLengthSeconds);
            quit.store(true);
            auto stopBeats = steady_clock::now();
            for (int tid = 0; tid < numThreads; tid++) enqdeqThreads[tid].join();
            lengthSec[irun] = (stopBeats-startBeats).count();
            startFlag.store(false);
            quit.store(false);
        }

        PE::write_transaction([&parray] () {
            // This free() causes Mnemosyne to crash, don't know why (yet)
            PE::pfree(parray);
            PE::template put_object<PersistentArrayInt<persist>>(PIDX_INT_ARRAY, nullptr);

        });
        // Accounting
        vector<long long> agg(numRuns);
        for (int irun = 0; irun < numRuns; irun++) {
            agg[irun] += ops[irun]*1000000000LL/lengthSec[irun];
        }
        // Compute the median. numRuns should be an odd number
        sort(agg.begin(),agg.end());
        auto maxops = agg[numRuns-1];
        auto minops = agg[0];
        auto medianops = agg[numRuns/2];
        auto delta = (long)(100.*(maxops-minops) / ((double)medianops));
        // Printed value is the median of the number of ops per second that all threads were able to accomplish (on average)
        std::cout << "Transactions/sec = " << medianops << "     delta = " << delta << "%   min = " << minops << "   max = " << maxops << "\n";
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
        //vector<int> threadList = { 1, 2, 4, 8, 16, 24, 32, 48, 56, 64 }; // for Cervino
        vector<int> threadList = { 1, 2, 4, 8 };
        vector<long> wordsPerTransList = { 1, 4, 8, 16, 32, 64, 128, 256 };
        const seconds testLength = 2s;   // 20s for the paper
        const int numRuns = 1;            // 5 runs for the paper

#ifdef PMDK_PTM
        std::cout << "If you're using PMDK on DRAM, don't forget to set export PMEM_IS_PMEM_FORCE=1\n";
#endif
        // SPS Benchmarks multi-threaded
        std::cout << "\n----- SPS Benchmark (multi-threaded integer array swap) -----\n";
        for (int nThreads : threadList) {
            for (int nWords : wordsPerTransList) {
                BenchmarkPersistency bench(nThreads);
                std::cout << "\n----- threads=" << nThreads << "   runs=" << numRuns << "   length=" << testLength.count() << "s   arraySize=" << arraySize << "   words/tx=" << nWords << " -----\n";
                bench.benchmarkSPS<romulus::Romulus, romulus::persist>(testLength, nWords, numRuns);
                bench.benchmarkSPS<romuluslog::RomulusLog, romuluslog::persist>(testLength, nWords, numRuns);
                bench.benchmarkSPS<romuluslr::RomulusLR, romuluslr::persist>(testLength, nWords, numRuns);
                bench.benchmarkSPS<romulusni::RomulusNI, romulusni::persist>(testLength, nWords, numRuns);
                //bench.benchmarkSPS<romulussf::RomulusSF, romulussf::persist>(testLength, nWords, numRuns);
#ifdef PMDK_PTM
                bench.benchmarkSPS<pmdk::PMDKTM, pmdk::persist>(testLength, nWords, numRuns);
#endif
#ifdef USE_MNEMOSYNE
                bench.benchmarkSPS<mnemosyne::Mnemosyne, mnemosyne::persist>(testLength, nWords, numRuns);
#endif
            }
        }

        /*
        // Array Benchmarks single threaded
        std::cout << "\n----- Array Benchmark -----\n";
		for (int nWords : wordsPerTransList) {
			BenchmarkPersistency bench(1);
			std::cout << "\n----- length=" << testLength.count() << "s   arraySize=" << arraySize << "   words/tx=" << nWords << " -----\n";
			bench.arrayBenchmark<romulus::Romulus, WrapperR>(testLength, nWords, numRuns);
			bench.arrayBenchmark<romuluslog::RomulusLog, WrapperRL>(testLength, nWords, numRuns);
#ifdef USE_MNEMOSYNE
			bench.arrayBenchmark<mnemosyne::Mnemosyne, WrapperM>(testLength, nWords, numRuns);
#endif
		}*/

/*
		// Wrapper Array Benchmarks multi-threaded
		std::cout << "\n----- Wrapper Array Benchmark (multi-threaded) -----\n";
        for (int nThreads : threadList) {
            for (int nWords : wordsPerTransList) {
                BenchmarkPersistency bench(nThreads);
                std::cout << "\n----- threads=" << nThreads << "   length=" << testLength.count() << "s   arraySize=" << arraySize << "   words/tx=" << nWords << " -----\n";
                bench.arrayBenchmark_rw<romulus::Romulus, WrapperR>(testLength, nWords, numRuns);
                bench.arrayBenchmark_rw<romuluslog::RomulusLog, WrapperRL>(testLength, nWords, numRuns);
                //bench.arrayBenchmark_rw<romuluslr::RomulusLR, WrapperR_LR>(testLength, nWords, numRuns);
#ifdef USE_MNEMOSYNE
                bench.arrayBenchmark_rw<mnemosyne::Mnemosyne, WrapperM>(testLength, nWords, numRuns);
#endif
            }
        }
*/
/*
        // Integer Array Benchmarks multi-threaded
        std::cout << "\n----- Integer Array Benchmark (multi-threaded) -----\n";
        for (int nThreads : threadList) {
            for (int nWords : wordsPerTransList) {
                BenchmarkPersistency bench(nThreads);
                std::cout << "\n----- threads=" << nThreads << "   length=" << testLength.count() << "s   arraySize=" << arraySize << "   words/tx=" << nWords << " -----\n";
                bench.integerArrayBenchmark_rw<romulus::Romulus, romulus::persist>(testLength, nWords, numRuns);
                bench.integerArrayBenchmark_rw<romuluslog::RomulusLog, romuluslog::persist>(testLength, nWords, numRuns);
                bench.integerArrayBenchmark_rw<romuluslr::RomulusLR, romuluslr::persist>(testLength, nWords, numRuns);
                //bench.integerArrayBenchmark_rw<romulus2flr::Romulus2FLR, romulus2flr::persist>(testLength, nWords, numRuns);
                //bench.integerArrayBenchmark_rw<romuluslogPWB::RomulusLogPWB, romuluslogPWB::persist>(testLength, nWords, numRuns);
#ifdef PMDK_STM
                bench.integerArrayBenchmark_rw<pmdk::PMDKTM, pmdk::persist>(testLength, nWords, numRuns);
#endif
#ifdef USE_MNEMOSYNE
                bench.integerArrayBenchmark_rw<mnemosyne::Mnemosyne, mnemosyne::persist>(testLength, nWords, numRuns);
#endif
            }
        }
*/
    }
};

#endif
