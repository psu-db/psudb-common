add_library(psu-ds Alias.h BitArray.h BloomFilter.h BTree.h dynarray.h LockedPriorityQueue.h PriorityQueue.h)
set_target_properties(psu-ds PROPERTIES LINKER_LANGUAGE CXX)
