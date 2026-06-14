// stacks.cpp

#include "cppu64.h"
#include <sched.h>
#include "spec.h"
#include "defs.h"


#define	FAULT_STACKSIZE		128

u64	dram_stack[SP_DRAM_STACK_SIZE64];
u64	schedule_stack[OS_SC_STACKSIZE/sizeof(u64)];
u64	idle_thread_stack[IDLE_STACKSIZE/sizeof(u64)];
u64	main_thread_stack[MAIN_STACKSIZE/sizeof(u64)];
//u64	fault_thread_stack[FAULT_STACKSIZE/sizeof(u64)];		// #defined in tengine.h
u64	boot_stack[STACKSIZE/sizeof(u64)];
u64	audio_stack[AUDIO_STACKSIZE/sizeof(u64)];
#ifndef MAKE_CART
u64	rmon_stack[RMON_STACKSIZE/sizeof(u64)];
#endif



/*
// in case i screw this up, here is the original configuration...
u64	dram_stack[SP_DRAM_STACK_SIZE64];
u64	schedule_stack[OS_SC_STACKSIZE/sizeof(u64)];
u64	idle_thread_stack[STACKSIZE/sizeof(u64)];
u64	fault_thread_stack[STACKSIZE/sizeof(u64)];
u64	main_thread_stack[MAIN_STACKSIZE/sizeof(u64)];
u64	boot_stack[STACKSIZE/sizeof(u64)];
u64	audio_stack[AUDIO_STACKSIZE/sizeof(u64)];
#ifndef MAKE_CART
u64	rmon_stack[RMON_STACKSIZE/sizeof(u64)];
#endif
*/
