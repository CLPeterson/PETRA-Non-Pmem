CXX=g++-7
BIN=bin
CFLAGS=-DPWB_IS_CLFLUSHOPT
# Possible options for PWB are:
# -DPWB_IS_STT		Emulates STT RAM with delays
# -DPWB_IS_PCM		Emulates PCM RAM with delays
# -DPWB_IS_CLFLUSH	pwb is a CLFLUSH and pfence/psync are nops      (Broadwell)
# -DPWB_IS_CLFLUSHOPT	pwb is a CLFLUSHOPT and pfence/psync are SFENCE (Kaby Lake) 
# -DPWB_IS_CLWB		pwb is a CLWB and pfence/psync are SFENCE       (SkyLake SP, or Ice Lake and beyond)
# -DPWB_IS_NOP		pwb/pfence/psync are nops. Used for shared memory persistence

# Make targets:
all: sps sets persistencyclean

sps: \
	$(BIN)/bench-sps \
	$(BIN)/bench-sps-asan \
#	$(BIN)/bench-sps-pmdk \
#	$(BIN)/bench-queues


sets: \
	$(BIN)/bench-sets-rom \
	$(BIN)/bench-sets-romlog \
	$(BIN)/bench-sets-romlog-O0 \
	$(BIN)/bench-sets-romlr \
	$(BIN)/bench-sets-romni \
	persistencyclean
#	$(BIN)/bench-sets-pmdk \
	persistencyclean \
	
experimental:
	persistencyclean \	


MYDEPS = \
	common/tm.h \
	common/pfences.h \
	common/ThreadRegistry.hpp \
	common/ThreadRegistry.cpp \
	romulus/Romulus.hpp \
	romulus/Romulus.cpp \
	romulus/malloc.cpp \
	romuluslog/RomulusLog.hpp \
	romuluslog/RomulusLog.cpp \
	romuluslog/malloc.cpp \
	romuluslr/RomulusLR.hpp \
	romuluslr/RomulusLR.cpp \
	romuluslr/malloc.cpp \
	romulusni/RomulusNI.hpp \
	romulusni/RomulusNI.cpp \
	romulusni/malloc.cpp \
	rwlocks/CRWWP.hpp \

DSDEPS = \
	datastructures/TMHashMap.hpp \
	datastructures/TMHashMapFixedSize.hpp \
	datastructures/TMLinkedListQueue.hpp \
	datastructures/TMLinkedListSet.hpp \
	datastructures/TMRedBlackBST.hpp \
	

INCLUDES = -I. $(CFLAGS)

SRCS = \
	common/ThreadRegistry.cpp \
	romulus/malloc.cpp \
	romulus/Romulus.cpp \
	romuluslog/RomulusLog.cpp \
	romuluslog/malloc.cpp \
	romuluslr/RomulusLR.cpp \
	romuluslr/malloc.cpp \
	romulusni/RomulusNI.cpp \
	romulusni/malloc.cpp \
	
	
SRCS_QUEUE = \
	common/ThreadRegistry.cpp \
	romuluslog/RomulusLog.cpp \
	romuluslog/malloc.cpp \
	romuluslr/RomulusLR.cpp \
	romuluslr/malloc.cpp \



#
# Benchmarks for sets
#
$(BIN)/bench-sets-rom: $(MYDEPS) $(DSDEPS) benchmarks/BenchmarkSets.hpp benchmarks/bench-sets.cpp
	$(CXX) -std=c++14 -fuse-ld=gold -g -O3 -DROMULUS_PTM benchmarks/bench-sets.cpp $(SRCS) $(INCLUDES) -o $(BIN)/bench-sets-rom -lpthread

$(BIN)/bench-sets-romlog: $(MYDEPS) $(DSDEPS) benchmarks/BenchmarkSets.hpp benchmarks/bench-sets.cpp
	$(CXX) -std=c++14 -fuse-ld=gold -g -O3 -DROMULUS_LOG_PTM benchmarks/bench-sets.cpp $(SRCS) $(INCLUDES) -o $(BIN)/bench-sets-romlog -lpthread

$(BIN)/bench-sets-romlog-O0: $(MYDEPS) $(DSDEPS) benchmarks/BenchmarkSets.hpp benchmarks/bench-sets.cpp
	$(CXX) -std=c++14 -fuse-ld=gold -g -O0 -DROMULUS_LOG_PTM benchmarks/bench-sets.cpp $(SRCS) $(INCLUDES) -o $(BIN)/bench-sets-romlog-O0 -lpthread

$(BIN)/bench-sets-romlr: $(MYDEPS) $(DSDEPS) benchmarks/BenchmarkSets.hpp benchmarks/bench-sets.cpp
	$(CXX) -std=c++14 -fuse-ld=gold -g -O3 -DROMULUS_LR_PTM benchmarks/bench-sets.cpp $(SRCS) $(INCLUDES) -o $(BIN)/bench-sets-romlr -lpthread

$(BIN)/bench-sets-romni: $(MYDEPS) $(DSDEPS) benchmarks/BenchmarkSets.hpp benchmarks/bench-sets.cpp
	$(CXX) -std=c++14 -fuse-ld=gold -g -O3 -DROMULUS_NI_PTM benchmarks/bench-sets.cpp $(SRCS) $(INCLUDES) -o $(BIN)/bench-sets-romni -lpthread

$(BIN)/bench-sets-pmdk: $(MYDEPS) $(DSDEPS) benchmarks/BenchmarkSets.hpp benchmarks/bench-sets.cpp pmdk/PMDKTM.hpp
	$(CXX) -std=c++14 -fuse-ld=gold -g -O3 -DPMDK_PTM benchmarks/bench-sets.cpp $(SRCS) $(INCLUDES) -o $(BIN)/bench-sets-pmdk -L/usr/local/lib -lpmemobj -lpthread

#$(BIN)/bench-sets-romsf: $(MYDEPS) $(DSDEPS) benchmarks/BenchmarkSets.hpp benchmarks/bench-sets.cpp
#	$(CXX) -std=c++14 -fuse-ld=gold -g -O3 -DROMULUS_SINGLE_FENCE_PTM benchmarks/bench-sets.cpp $(SRCS) $(INCLUDES) -o $(BIN)/bench-sets-romsf -lpthread
#
#$(BIN)/bench-sets-crwwptm: $(MYDEPS) $(DSDEPS) benchmarks/BenchmarkSets.hpp benchmarks/bench-sets.cpp
#	$(CXX) -std=c++14 -fuse-ld=gold -g -O3 -DCRWWP_STM benchmarks/bench-sets.cpp $(SRCS) $(INCLUDES) -o $(BIN)/bench-sets-crwwptm -lpthread



#
# Speedup benchmarks, to make a plot like figure 5 of Mnemosyne
#
$(BIN)/bench-speedup-romlog: $(MYDEPS) $(DSDEPS) benchmarks/BenchmarkSpeedup.hpp benchmarks/bench-speedup.cpp
	$(CXX) -std=c++14 -fuse-ld=gold -g -O3 -DROMULUS_LOG_PTM benchmarks/bench-speedup.cpp $(SRCS) $(INCLUDES) -o $(BIN)/bench-speedup-romlog -lpthread

$(BIN)/bench-speedup-pmdk: $(MYDEPS) $(DSDEPS) benchmarks/BenchmarkSpeedup.hpp benchmarks/bench-speedup.cpp pmdk/PMDKTM.hpp
	$(CXX) -std=c++14 -fuse-ld=gold -g -O3 -DPMDK_PTM benchmarks/bench-speedup.cpp $(SRCS) $(INCLUDES) -o $(BIN)/bench-speedup-pmdk -L/usr/local/lib -lpmemobj -lpthread


#
# Benchmarks for SPS
#
$(BIN)/bench-sps: $(MYDEPS) $(DSDEPS) benchmarks/BenchmarkSPS.hpp benchmarks/bench-sps.cpp
	$(CXX) -std=c++14 -fuse-ld=gold -g -O3 benchmarks/bench-sps.cpp $(SRCS) $(INCLUDES) -o $(BIN)/bench-sps -lpthread

$(BIN)/bench-queues: $(MYDEPS) $(DSDEPS) benchmarks/PBenchmarkQueues.hpp benchmarks/pq-ll-enq-deq.cpp
	$(CXX) -std=c++14 -fuse-ld=gold -g -O3 benchmarks/pq-ll-enq-deq.cpp $(SRCS_QUEUE) $(INCLUDES) -o $(BIN)/bench-queues -lpthread
	
# Same as before but with pmem (needs pmem/pmdk library)
$(BIN)/bench-sps-pmdk: $(MYDEPS) $(DSDEPS) benchmarks/BenchmarkSPS.hpp benchmarks/bench-sps.cpp pmdk/PMDKTM.hpp
	$(CXX) -std=c++14 -fuse-ld=gold -g -O3 $(SRCS) $(INCLUDES) -DPMDK_PTM benchmarks/bench-sps.cpp -o $(BIN)/bench-sps-pmdk -L/usr/local/lib -lpmemobj -lpthread

$(BIN)/bench-sps-asan: $(MYDEPS) $(DSDEPS) benchmarks/BenchmarkSPS.hpp benchmarks/bench-sps.cpp 
	$(CXX) -std=c++14 -fuse-ld=gold -g -fsanitize=address benchmarks/bench-sps.cpp $(SRCS) $(INCLUDES) -o $(BIN)/bench-sps-asan -lpthread



persistencyclean:
	rm -f /dev/shm/romulus_shared
	rm -f /dev/shm/romuluslog_shared
	rm -f /dev/shm/romuluslr_shared
	rm -f /dev/shm/romulussf_shared
	rm -f /dev/shm/romulusni_shared
	rm -f /dev/shm/romulus2flr_shared
	rm -f /dev/shm/psegments/*
	rm -f /dev/shm/pmdk_shared


# Use this to run all the benchmarks in one go and save the logs in files at the home folder
run-benchmarks:
	@echo "Make sure you've set in datastructures/BenchmarkSets.hpp the right threadList, time, number of runs, number of elements, and enabled data structures"
	make sets bench persistencyclean
	@echo "Starting to run benchamrks. This may take several hours..."
	bin/bench-sets-rom >       ~/cervino-rom-1K-1run-O3.txt
	bin/bench-sets-romlog >    ~/cervino-romlog-1K-1run-O3.txt
	bin/bench-sets-romlog-O0 > ~/cervino-romlog-1K-1run-O0.txt
	bin/bench-sets-romlr >     ~/cervino-romlr-1K-1run-O3.txt
	bin/bench-sets-pmdk >      ~/cervino-pmdk-1K-1run-O3.txt
#	bin/bench-sets-romsf >     ~/cervino-romsf-1K-1run-O3.txt
#	./bench >                  ~/aws-bench-all.txt


# Use this on AWS c5.9xlarge
aws-benchmarks:
	export PMEM_IS_PMEM_FORCE=1
	@echo " "
	@echo "Make sure you've set in datastructures/BenchmarkSets.hpp the right threadList, time, number of runs, number of elements, and enabled data structures"
	make sets bench $(BIN)/bench-speedup-romlog $(BIN)/bench-speedup-pmdk persistencyclean
	@echo " "
	@echo "Did you compile everything with -DCLWB_CPU ?"
	@echo "Starting to run benchamrks. This may take several hours..."
	bin/bench-sets-romlog >    ~/aws-c59xlarge-sets-romlog-1K-1run-O3.txt
	bin/bench-sets-romlr >     ~/aws-c59xlarge-sets-romlr-1K-1run-O3.txt
	bin/bench-sets-pmdk >      ~/aws-c59xlarge-sets-pmdk-1K-1run-O3.txt
	bin/bench-speedup-romlog > ~/aws-c59xlarge-speedup-romlog.txt
	bin/bench-speedup-pmdk >   ~/aws-c59xlarge-speedup-pmdk.txt
	@echo "Now go and run the Mnemosyne benchmarks manually..."



# We also clean the persistency files so that on a new build we get a clean slate
clean: persistencyclean
	rm -f bench
	rm -f bench-asan
	rm -f bench-pmdk
	rm -f bin/*
	
		
