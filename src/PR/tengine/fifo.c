// fifo.cpp

#include "cppu64.h"
#include "tengine.h"

#ifdef USE_FIFO
u64	fifo_buffer[FIFO_SIZE/sizeof(u64)];
#endif
