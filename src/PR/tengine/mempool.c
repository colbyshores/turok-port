// mempool.c

#include "cppu64.h"
#include "tengine.h"

// memory pool for dynamic allocations
u32 dynamic_memory_pool[MEMORY_POOL_SIZE/sizeof(u32)];
