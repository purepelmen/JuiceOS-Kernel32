#include "drivers/screen.h"
#include "stdio.h"
#include "kernel.h"
#include "heap.h"

void InitializeHeap(void)
{
    currentHeapValue = (uint32) &end;
    heapStartValue = currentHeapValue;

    PrintLog("Heap was initialised.\n");
    Print("Heap was initialised.\n");
}

void reset_heap(void)
{
    currentHeapValue = heapStartValue;
    PrintLog("Heap resetting has been completed.\n");
}

uint8* malloc(uint32 size)
{
    uint32 tmp = currentHeapValue;
    currentHeapValue += (uint32) size;
    return (uint8*) tmp;
}