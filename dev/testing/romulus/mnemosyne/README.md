### Mnemosyne ###

There are a few files needed to compile this with Mnemosyne, and they are Mnemosyne specific:
  pvar.c
  pvar.h
  SConscript
  
You must first install Mnemosyne. Instructions can be found on this link https://github.com/snalli/mnemosyne-gcc
but here is the summary of the steps for Ubuntu systems:

sudo apt-get install libconfig-dev libconfig9 libelf-dev elfutils libevent-dev scons libboost-all-dev cmake libnuma-dev
git clone https://github.com/snalli/mnemosyne-gcc
cd mnemosyne-gcc
cd usermode/library/pmalloc/include/alps
mkdir build
cd build
cmake .. -DTARGET_ARCH_MEM=CC-NUMA -DCMAKE_BUILD_TYPE=Release
make -j10
cd ../../../../..
scons --build-example=simple
./build/examples/simple/simple

Now increase the size of the log in entry RW_SET_SIZE of usermode/library/configuration/default/mtm.py to 100x more.
Then build our benchmarks (with Mnemosyne) like this:
cp -R /nvram/code/romulus/ examples/  
scons --build-example=romulus 
rm -rf /dev/shm/psegments
rm -f /dev/shm/romulus*
./build/examples/romulus/bench
./build/examples/romulus/bench-sets

If you don't want the pesky logs, go to: 
  usermode/library/pmalloc/src/heap.cc
and uncomment the log line 21, alps::init_log(dbgopt);
then recompile.


### Memcached benchmarks ###
(Not yet on the repo)
Build with:
  scons --build-bench=memcached
Start up memcached with one of these:
  ./build/bench/memcached/memcached-1.2.4-base/memcached -u root -p 11211 -l 127.0.0.1 -t 4 &
  ./build/bench/memcached/memcached-1.2.4-mtm/memcached -u root -p 11211 -l 127.0.0.1 -t 4 &
  rm /dev/shm/romulus_log_shared
  ./build/bench/memcached/memcached-1.2.4-rom/memcached -u root -p 11211 -l 127.0.0.1 -t 4 &
Run the benchmarks with:  
  ./run_memslap.sh


### Vacation benchmarks ###
(Not yet on the repo)
These benchmarks are based on:
  https://github.com/kozyraki/stamp
  
Build and run the 'vacation' benchmark with Mnemosyne:
  scons --build-bench=stamp-kozy
  export LD_LIBRARY_PATH=`pwd`/library:$LD_LIBRARY_PATH
  rm -rf /dev/shm/psegments
  ./build/bench/stamp-kozy/vacation/vacation -c0 -n1 -r65536 -q100
  ./build/bench/stamp-kozy/vacation/vacation -t400000 -c4 -n10 -r65536 -q90 -u100

Build and run the 'vacation' benchmark with RomulusLR:
  scons --build-bench=stamp-rom
  export LD_LIBRARY_PATH=`pwd`/library:$LD_LIBRARY_PATH
  rm /tmp/romuluslr_shared
  ./build/bench/stamp-rom/vacation/vacation -c0 -n1 -r65536 -q100
  ./build/bench/stamp-rom/vacation/vacation -t400000 -c4 -n10 -r65536 -q90 -u100
