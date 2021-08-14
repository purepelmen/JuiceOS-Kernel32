#include "stdio.h"
#include "kernel.h"
#include "heap.h"

void init_heap(void) {
    currentHeapValue = (uint32) &end;
    heapStartValue = currentHeapValue;
    print_log("Heap was initialised.\n");
}

void reset_heap(void) {
    currentHeapValue = heapStartValue;
    print_log("Heap resetting has been completed.\n");
}

uint8* malloc(uint32 size) {
    uint32 tmp = currentHeapValue;
    currentHeapValue += (uint32) size;
    return (uint8*) tmp;
}