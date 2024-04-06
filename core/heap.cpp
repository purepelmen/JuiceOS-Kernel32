#include "drivers/screen.hpp"
#include "stdlib.hpp"
#include "kernel.hpp"
#include "string.hpp"
#include "heap.hpp"

uint32 head_start_value;
uint32 current_heap_value;

void init_heap(void)
{
    current_heap_value = (uint32) &end;
    head_start_value = current_heap_value;

    kernel_print_log("Heap was initialized.\n");
}

void reset_heap(void)
{
    current_heap_value = head_start_value;
    kernel_print_log("Heap resetting has been completed.\n");
}

void* malloc(uint32 size)
{
    uint32 tmp = current_heap_value;
    current_heap_value += (uint32) size;
    return (uint8*) tmp;
}

void* malloc_pg_aligned(uint32 size)
{
    if(current_heap_value & 0xFFFFF000)
    {
        current_heap_value &= 0xFFFFF000;
        current_heap_value += 0x1000;
    }

    return (void*) malloc(size);
}
