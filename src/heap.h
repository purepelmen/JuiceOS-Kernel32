#ifndef C_HEAP_LIB
#define C_HEAP_LIB

#include "stdlib.h"

uint32 heapStartValue;
uint32 currentHeapValue;
extern uint32 end;

/* Initialize heap */
void InitializeHeap(void);

/* Reset heap */
void reset_heap(void);

/* Allocate a block of the memory */
uint8* malloc(uint32 size);

#endif