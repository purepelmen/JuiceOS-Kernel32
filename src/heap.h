#ifndef C_HEAP_LIB
#define C_HEAP_LIB

#include "stdio.h"

uint32 heapStartValue;
uint32 currentHeapValue;
extern uint32 end;

/* Initialize the heap */
void init_heap(void);

/* Reset the heap */
void reset_heap(void);

/* Allocate a block of the memory with given size */
uint8* malloc(uint32 size);

#endif