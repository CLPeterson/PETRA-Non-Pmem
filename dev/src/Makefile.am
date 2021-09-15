
bin_PROGRAMS = trans transRomulus transOneFile
# bin_PROGRAMS = transRomulus

SUBDIRS = common/fraser

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/intel/tbb/lib/intel64/gcc4.7

trans_SOURCES = bench/main.cc \
				common/allocator.cc\
				common/timehelper.cc\
				common/assert.cc\
				lftt/list/translist.cc\
				lftt/skiplist/transskip.cc\
				lftt/mdlist/txmdlist.cc\
				lftt/map/transmap.cc\
				durabletxn/dtx.cc\
				boostinglist/boostinglist.cc\
				lockfreelist/lockfreelist.cc\
				boostinglist/lockkey.cc

trans_CPPFLAGS = $(WARN_FLAGS) -std=c++14 -pthread\
				 -DINTEL -DCACHE_LINE_SIZE=`getconf LEVEL1_DCACHE_LINESIZE`\
				 -I/opt/intel/tbb/include -I$(top_builddir)/src -I$(top_srcdir)/src  

trans_LDFLAGS = -L/opt/intel/tbb/lib/intel64/gcc4.7

# uncomment this for a pmem system
#PMEM_LIB_FLAGS = -lpmemobj -lpmem

trans_LDADD =  $(PMEM_LIB_FLAGS) -lrt -ltbb -ltcmalloc common/fraser/libfd.la


transRomulus_SRCS = \
	romulus/commonromulus/ThreadRegistry.cpp \
	romulus/romulus/malloc.cpp \
	romulus/romulus/Romulus.cpp \
	romulus/romuluslog/RomulusLog.cpp \
	romulus/romuluslog/malloc.cpp \
	romulus/romuluslr/RomulusLR.cpp \
	romulus/romuluslr/malloc.cpp \
	romulus/romulusni/RomulusNI.cpp \
	romulus/romulusni/malloc.cpp

# transOneFile_SRCS = \
# 	OneFile/common/ThreadRegistry.cpp \
# 	OneFile/romulus/malloc.cpp \
# 	romulus/romulus/Romulus.cpp \
# 	romulus/romuluslog/RomulusLog.cpp \
# 	romulus/romuluslog/malloc.cpp \
# 	romulus/romuluslr/RomulusLR.cpp \
# 	romulus/romuluslr/malloc.cpp \
# 	romulus/romulusni/RomulusNI.cpp \
# 	romulus/romulusni/malloc.cpp	

transRomulus_INCLUDES = -I$(top_builddir)/src/romulus $(CFLAGS)

transOneFile_INCLUDES = -I$(top_builddir)/src/OneFile $(CFLAGS)

# transRomulus_CFLAGS=-DPWB_IS_CLFLUSHOPT
# 				 -DPWB_IS_CLFLUSHOPT
#	-DROMULUS_PTM
#	-DROMULUS_LOG_PTM
#	-DROMULUS_LR_PTM
#	-DROMULUS_NI_PTM
# make CPPFLAGS="-DPWB_IS_CLFLUSHOPT"

transRomulus_CPPFLAGS = $(WARN_FLAGS) -std=c++14 \
				 -DINTEL -DCACHE_LINE_SIZE=`getconf LEVEL1_DCACHE_LINESIZE`\
				 $(transRomulus_INCLUDES) \
				 -fuse-ld=gold

transOneFile_CPPFLAGS = $(WARN_FLAGS) -std=c++14 \
				 -DINTEL -DCACHE_LINE_SIZE=`getconf LEVEL1_DCACHE_LINESIZE`\
				 $(transOneFile_INCLUDES) \
				 -fuse-ld=gold
 
transRomulus_LDFLAGS = -L/opt/intel/tbb/lib/intel64/gcc4.7/

transRomulus_LDADD = $(PMEM_LIB_FLAGS) -lrt -ltbb -lpthread

transRomulus_SOURCES = bench/RomulusMain.cc \
				common/timehelper.cc\
				romulus/commonromulus/ThreadRegistry.cpp \
				romulus/romulus/Romulus.cpp \
				romulus/romulus/malloc.cpp \
				romulus/romuluslog/RomulusLog.cpp \
				romulus/romuluslog/malloc.cpp \
				romulus/romuluslr/RomulusLR.cpp \
				romulus/romuluslr/malloc.cpp \
				romulus/romulusni/RomulusNI.cpp \
				romulus/romulusni/malloc.cpp

transOneFile_SOURCES = bench/OneFileMain.cc \
				common/timehelper.cc


transOneFile_LDADD = $(PMEM_LIB_FLAGS) -ltbb	#CORRECTNESS ANNOTATIONS added -ltbb		
