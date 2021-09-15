/*
 * main.cpp
 *
 *  Created on: Apr 23, 2016
 *      Author: pramalhe
 */
#include <thread>
#include <string>

#include "benchmarks/BenchmarkSPS.hpp"

// g++ -std=c++14 main.cpp -I../include
int main(int argc, char *argv[]){

    BenchmarkPersistency::allThroughputTests();
    return 0;
}

