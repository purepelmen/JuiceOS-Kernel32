#ifndef C_HEAP_LIB
#define C_HEAP_LIB

#include "stdlib.hpp"

extern uint32 head_start_value;
extern uint32 current_heap_value;
extern uint32 end;

/* Initialize heap */
void init_heap(void);

/* Reset heap */
void reset_heap(void);

/* Allocate a block of the memory */
void* malloc(uint32 size);

/* Allocate an 4k aligned block of memory */
void* malloc_pg_aligned(uint32 size);

#endif