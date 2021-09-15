# This is a script to exectue the benchmarks for Mnemosyne because Mnemosyne may crash on each iteration


rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=8
rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=64
rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=256
rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=1024

rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=8 nThreads=2
rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=64 nThreads=2
rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=256 nThreads=2
rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=1024 nThreads=2

rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=8 nThreads=4
rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=64 nThreads=4
rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=256 nThreads=4
rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=1024 nThreads=4

rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=8 nThreads=16
rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=64 nThreads=16
rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=256 nThreads=16
rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=1024 nThreads=16

rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=8 nThreads=30
rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=64 nThreads=30
rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=256 nThreads=30
rm /dev/shm/psegments/*
./build/examples/romulus/bench-speedup-single value=1024 nThreads=30
