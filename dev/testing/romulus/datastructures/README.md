# Persistent Data structures #
This folder contains three persistent data structures:
  TMHashMap.hpp           - A resizable Hash Map (uses a global counter, which affects disjoin-parallelism) 
  TMHashMapFixed.hpp      - A fixed-size Hash Map
  TMLinkedListQueue.hpp   - A memory unbounded queue, backed by a linked list
  TMLinkedListSet.hpp     - A Set based on a linked list
  TMRedBlackBST.hpp       - A Map backed by a Red-Black Binary Search Tree
  
These data structures were written to be "portable" across different STMs or PTMs.
The common/tm.h file contains definitions of the macros used in these data structures. 
If you need to benchmark other PTMs just add the corresponding macros to tm.h


