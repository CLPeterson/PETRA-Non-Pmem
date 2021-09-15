# This is a script to exectue the benchmarks for Mnemosyne because Mnemosyne may crash on each iteration
# Before running this script, you may want to do the following modifications to your Mnemosyne repo (though everything will be slower in that case):
# 1) Change LOG_NUM to 65 in usermode/library/mcore/src/log/mgr.c
# 2) Change LOG_POOL_SIZE to (66*16*1024*1024+66*32*64) in usermode/library/mcore/include/pregionlayout.h;


rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=1 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=2 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=4 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=8 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=16 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=24 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=30 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=32 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=48 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=56 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=64 ratio=1000 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=1 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=2 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=4 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=8 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=16 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=24 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=30 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=32 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=48 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=56 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=64 ratio=500 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=1 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=2 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=4 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=8 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=16 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=24 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=30 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=32 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=48 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=56 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=64 ratio=100 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=1 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=2 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=4 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=8 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=16 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=24 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=30 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=32 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=48 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=56 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=64 ratio=10 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=1 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=2 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=4 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=8 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=16 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=24 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=30 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=32 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=56 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=48 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=64 ratio=1 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=1 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=2 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=4 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=8 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=16 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=24 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=30 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=32 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=48 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=56 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=0 nThreads=64 ratio=0 numElements=1000 testLength=20 numRuns=1



rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=1 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=2 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=4 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=8 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=16 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=24 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=30 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=32 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=48 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=56 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=64 ratio=1000 numElements=1000 testLength=20 numRuns=1


rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=1 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=2 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=4 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=8 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=16 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=24 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=30 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=32 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=48 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=56 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=64 ratio=500 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=1 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=2 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=4 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=8 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=16 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=24 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=30 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=32 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=48 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=56 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=64 ratio=100 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=1 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=2 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=4 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=8 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=16 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=24 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=30 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=32 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=48 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=56 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=64 ratio=10 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=1 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=2 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=4 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=8 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=16 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=24 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=30 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=32 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=48 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=56 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=64 ratio=1 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=1 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=2 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=4 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=8 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=16 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=24 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=30 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=32 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=48 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=56 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=1 nThreads=64 ratio=0 numElements=1000 testLength=20 numRuns=1







rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=1 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=2 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=4 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=8 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=16 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=24 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=30 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=32 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=48 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=56 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=64 ratio=1000 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=1 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=2 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=4 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=8 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=16 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=24 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=30 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=32 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=48 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=56 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=64 ratio=500 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=1 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=2 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=4 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=8 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=16 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=24 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=30 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=32 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=48 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=56 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=64 ratio=100 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=1 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=2 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=4 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=8 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=16 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=24 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=30 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=32 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=48 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=56 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=64 ratio=10 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=1 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=2 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=4 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=8 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=16 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=24 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=30 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=32 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=48 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=56 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=64 ratio=1 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=1 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=2 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=4 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=8 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=16 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=24 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=30 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=32 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=48 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=56 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=2 nThreads=64 ratio=0 numElements=1000 testLength=20 numRuns=1


rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=1 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=2 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=4 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=8 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=16 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=24 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=30 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=32 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=48 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=56 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=64 ratio=1000 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=1 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=2 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=4 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=8 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=16 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=24 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=30 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=32 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=48 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=56 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=64 ratio=500 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=1 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=2 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=4 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=8 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=16 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=24 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=30 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=32 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=48 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=56 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=64 ratio=100 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=1 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=2 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=4 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=8 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=16 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=24 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=30 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=32 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=48 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=56 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=64 ratio=10 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=1 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=2 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=4 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=8 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=16 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=24 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=30 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=32 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=56 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=48 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=64 ratio=1 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=1 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=2 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=4 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=8 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=16 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=24 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=30 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=32 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=48 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=56 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=3 nThreads=64 ratio=0 numElements=1000 testLength=20 numRuns=1



rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=1 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=2 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=4 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=8 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=16 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=24 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=30 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=32 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=48 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=56 ratio=1000 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=64 ratio=1000 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=1 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=2 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=4 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=8 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=16 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=24 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=30 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=32 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=48 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=56 ratio=500 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=64 ratio=500 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=1 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=2 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=4 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=8 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=16 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=24 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=30 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=32 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=48 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=56 ratio=100 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=64 ratio=100 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=1 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=2 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=4 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=8 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=16 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=24 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=30 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=32 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=48 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=56 ratio=10 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=64 ratio=10 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=1 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=2 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=4 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=8 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=16 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=24 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=30 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=32 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=56 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=48 ratio=1 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=64 ratio=1 numElements=1000 testLength=20 numRuns=1

rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=1 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=2 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=4 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=8 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=16 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=24 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=30 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=32 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=48 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=56 ratio=0 numElements=1000 testLength=20 numRuns=1
rm /dev/shm/psegments/*
./build/examples/romulus/bench-sets-single class=4 nThreads=64 ratio=0 numElements=1000 testLength=20 numRuns=1



