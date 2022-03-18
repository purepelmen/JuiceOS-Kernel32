#include "drivers/screen.hpp"

#include "stdlib.hpp"
#include "kernel.hpp"
#include "heap.hpp"
#include "string.hpp"

uint32 heapStartValue;
uint32 currentHeapValue;

void InitializeHeap(void)
{
    currentHeapValue = (uint32) &end;
    heapStartValue = currentHeapValue;

    PrintLog(string("Heap was initialised.\n"));
}

void reset_heap(void)
{
    currentHeapValue = heapStartValue;
    PrintLog(string("Heap resetting has been completed.\n"));
}

uint8* malloc(uint32 size)
{
    uint32 tmp = currentHeapValue;
    currentHeapValue += (uint32) size;
    return (uint8*) tmp;
}