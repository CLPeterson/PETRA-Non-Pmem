# PETRA-Non-Pmem
## Build:
Install dependencies:

`sudo apt-get install libboost-all-dev libgoogle-perftools-dev libtool m4 automake cmake libtbb-dev libgsl0-dev` <br />
`mkdir compile` <br />
`cd dev` <br />
`bash bootstrap.sh` <br />
`cd ../compile` <br />
`../dev/configure` <br />
`make CPPFLAGS="-DUSE_DRAM_ALLOCATOR -DPWB_IS_CLFLUSH -DROMULUS_LR_PTM -O0 -g"` <br />

## Options:
setType: { 0="TransList" , 1="TransSkip", 2="TransMDList", 3="TransMap" } <br />
numThread: Number of Threads <br />
testSize: Test Size <br />
tranSize: Transaction Size <br />
keyRange: Key Range <br />
insertion: Insertion Percent <br />
deletion: Deletion Percent <br />
-v: verbose <br />

## Run:
`cd src/petra/compile` <br />
`./src/trans <setType> <numThread> <testSize> <tranSize> <keyRange> <insertion> <deletion>` 

For example, to test TransSkip with 16 threads, test size is 100, transaction size is 4 operations, key range is 1000, insert percent is 50, and delete percent is 25 with verbose flag: <br />
`./src/trans 1 16 100 4 1000 50 25 -v`

## Run Small Examples for PETRA:
TransList with 16 threads, test size is 100, transaction size is 4 operations, key range is 1000, insert percent is 50, and delete percent is 25 with verbose flag: <br />
`./src/trans 0 16 100 4 1000 50 25 -v` <br />

TransSkip with 16 threads, test size is 100, transaction size is 4 operations, key range is 1000, insert percent is 50, and delete percent is 25 with verbose flag: <br />
`./src/trans 1 16 100 4 1000 50 25 -v` <br />

TransMDList with 16 threads, test size is 100, transaction size is 4 operations, key range is 1000, insert percent is 50, and delete percent is 25 with verbose flag: <br />
`./src/trans 2 16 100 4 1000 50 25 -v` <br />

TransMap with 16 threads, test size is 100, transaction size is 4 operations, key range is 1000, insert percent is 50, and delete percent is 25 with verbose flag: <br />
`./src/trans 3 16 100 4 1000 50 25 -v` <br />

## Run Small Examples for Romulus:
TransList with 16 threads, test size is 100, transaction size is 4 operations, key range is 1000, insert percent is 50, and delete percent is 25 with verbose flag: <br />
`./src/transRomulus 0 16 100 4 1000 50 25 -v` <br />

TransSkip with 16 threads, test size is 100, transaction size is 4 operations, key range is 1000, insert percent is 50, and delete percent is 25 with verbose flag: <br />
`./src/transRomulus 1 16 100 4 1000 50 25 -v` <br />

TransMDList with 16 threads, test size is 100, transaction size is 4 operations, key range is 1000, insert percent is 50, and delete percent is 25 with verbose flag: <br />
`./src/transRomulus 2 16 100 4 1000 50 25 -v` <br />

TransMap with 16 threads, test size is 100, transaction size is 4 operations, key range is 1000, insert percent is 50, and delete percent is 25 with verbose flag: <br />
`./src/transRomulus 3 16 100 4 1000 50 25 -v` <br />

## Run Small Examples for OneFile:
TransList with 16 threads, test size is 100, transaction size is 4 operations, key range is 1000, insert percent is 50, and delete percent is 25 with verbose flag: <br />
`./src/transOneFile 0 16 100 4 1000 50 25 -v` <br />

TransSkip with 16 threads, test size is 100, transaction size is 4 operations, key range is 1000, insert percent is 50, and delete percent is 25 with verbose flag: <br />
`./src/transOneFile 1 16 100 4 1000 50 25 -v` <br />

TransMDList with 16 threads, test size is 100, transaction size is 4 operations, key range is 1000, insert percent is 50, and delete percent is 25 with verbose flag: <br />
`./src/transOneFile 2 16 100 4 1000 50 25 -v` <br />

TransMap with 16 threads, test size is 100, transaction size is 4 operations, key range is 1000, insert percent is 50, and delete percent is 25 with verbose flag: <br />
`./src/transOneFile 3 16 100 4 1000 50 25 -v` <br />
