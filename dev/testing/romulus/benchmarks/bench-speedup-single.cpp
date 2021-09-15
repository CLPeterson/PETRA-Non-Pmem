#include <string>
#include <fstream>
#include <string.h>

#include "../benchmarks/BenchmarkSpeedup.hpp"

using namespace std;
using namespace chrono;

// Execute a single iteration and return the result
// Call like this:
// ./bench-sets-single class=0 nThreads=2 ratio=10 numElements=1000  testLength=20 numRuns=1
//
int main(int argc, char *argv[]) {
    int value = 8;
    int iclass = 3;
    int nThreads = 1;
    int numRuns = 1;
    int ratio = 1000;
    int numElements = 100; // WARNING: Mnemosyne will crash with more, so 100 is all we put for the moment
    seconds testLength = 20s;
    bool dedicated = false;

    for (int i = 1; i < argc; i++) {
        string param{argv[i]};
        if (param.find("value=") != std::string::npos) value = atoi(argv[i]+strlen("value="));
        else if (param.find("class=") != std::string::npos) iclass = atoi(argv[i]+strlen("class="));
        else if (param.find("nThreads=") != std::string::npos) nThreads = atoi(argv[i]+strlen("nThreads="));
        else if (param.find("ratio=") != std::string::npos) ratio = atoi(argv[i]+strlen("ratio="));
        else if (param.find("numElements=") != std::string::npos) numElements = atoi(argv[i]+strlen("numElements="));
        else if (param.find("testLength=") != std::string::npos) testLength = 1s*atoi(argv[i]+strlen("testLength="));
        else if (param.find("numRuns=") != std::string::npos) numRuns = atoi(argv[i]+strlen("numRuns="));
        else cout << "unrecognized parameter: " << argv[i] << "\n";
    }

    BenchmarkSpeedup::singleTest(value, iclass, nThreads, ratio, numElements, testLength, numRuns);
    return 0;
}

