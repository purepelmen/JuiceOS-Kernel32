#include "drivers/screen.hpp"

#include "stdlib.hpp"
#include "kernel.hpp"
#include "heap.hpp"
#include "string.hpp"

uint32 head_start_value;
uint32 current_heap_value;

void init_heap(void)
{
    current_heap_value = (uint32) &end;
    head_start_value = current_heap_value;

    kernel_print_log("Heap was initialised.\n");
}

void reset_heap(void)
{
    current_heap_value = head_start_value;
    kernel_print_log("Heap resetting has been completed.\n");
}

uint8* malloc(uint32 size)
{
    uint32 tmp = current_heap_value;
    current_heap_value += (uint32) size;
    return (uint8*) tmp;
}