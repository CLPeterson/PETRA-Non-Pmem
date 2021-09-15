# PETRA: Persistent Transactional Non-blocking Linked Data Structures

## Organization

`testing/`: Codes and scripts for tests

`src/lftt`: The code transformed from LFTT
`src/Romulus`: The code transformed from Romulus
`src/oneFile`: The code transformed from OneFile

`src/durabletxn`: Code to support Durable transactions.


## Build Instructions

### Dependencies from LFTT
#### INSTALL DEPENDENCIES
`sudo apt-get install libboost-all-dev libgoogle-perftools-dev libtool m4 automake cmake libtbb-dev libgsl0-dev`

### Dependencies related to persistent memory

https://pmem.io/pmdk/libpmem/

https://pmem.io/pmdk/libpmemobj/

### Configure
`mkdir petra`

`cd petra`

`git clone <github url>`

`(change the folder name to "dev")`

`cd dev`

`bash bootstrap.sh`

`create a folder named "compile" at the root of the petra folder`

`cd ../compile`

`../dev/configure`


### Compile
`cd compile`

`make <options>`


#### Options

##### Memory Allocator
 1) DRAM (Default)
	`-DUSE_DRAM_ALLOCATOR`
	
 2) Memory Mapped Files (Disk)
 	`-DUSE_MMAP_ALLOCATOR`
	
 3) Persistent Memory
	`-DUSE_PMEM_ALLOCATOR`

##### FLUSH Type
 1) CLFLUSH
	`-DPWB_IS_CLFLUSH`
 2) CLFLUSHOPT
 	`-DPWB_IS_CLFLUSHOPT`
 3) CLWB
 	`-DPWB_IS_CLWB`
	
##### Related Work Build Options
 1) Romulus
 	`-DROMULUS_LR_PTM`
 2) PMDK
 	`-DPMDK_PTM`

#### Example
	`make CPPFLAGS="-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLWB -DROMULUS_LR_PTM"`

### Run
`cd compile/src`
`./trans <options for the run>`
#### Options
   ` setType: { 0="TransList" , 
               1="TransSkip",
               2="TransMDList",
               3="TransMap"
	      }`
	      
    `numThread`
    
    `testSize`
    
    `ranSize`
    
    `keyRange`
    
    `insertion percentage`
    
    `deletion percentage`
    
    `update percentage` # if applicable
	
###### Note
Find ratio = 100 - (insertion + deletion + update)

