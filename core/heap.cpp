#include "drivers/screen.hpp"
#include "stdlib.hpp"
#include "kernel.hpp"
#include "string.hpp"
#include "heap.h"

static uint8* heap_start_value;
static uint8* current_heap_value;

extern uint32 end;

namespace kheap
{
    void init()
    {
        current_heap_value = (uint8*) &end;
        heap_start_value = current_heap_value;

        kernel_print_log("Heap was initialized.\n");
    }

    void reset()
    {
        current_heap_value = heap_start_value;
        kernel_print_log("Heap resetting has been completed.\n");
    }

    void* alloc(uint32 size)
    {
        uint8* tmp = current_heap_value;
        current_heap_value += size;

        return tmp;
    }

    void* alloc_pg_aligned(uint32 size)
    {
        uint32 currentPtr = (uint32)current_heap_value;
        if(currentPtr & 0xFFFFF000)
        {
            currentPtr &= 0xFFFFF000;
            currentPtr += 0x1000;
        }
        current_heap_value = (uint8*) currentPtr;

        return alloc(size);
    }

    uint32 get_allocated_size()
    {
        return (uint32) (current_heap_value - heap_start_value);
    }

    uint32 get_system_mem_size()
    {
        return (uint32) (heap_start_value - 0x100000);
    }

    void *get_location_ptr()
    {
        return heap_start_value;
    }
}
