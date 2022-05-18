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
uint8* malloc(uint32 size);

#endif