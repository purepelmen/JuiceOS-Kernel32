#include "stdlib.h"
#include "string.h"
#include "kernel.h"
#include "heap.h"

extern uint32 end;

/* The heap.h new[] template function uses it, because sometimes new[] requires the definition of this. */
extern "C" void __cxa_throw_bad_array_new_length()
{
    RAISE_ERROR("__cxa_throw_bad_array_new_length happened")
}

namespace kheap
{
    static uint8* heap_start_value;
    static uint8* current_heap_value;

    void init()
    {
        current_heap_value = (uint8*) &end;
        heap_start_value = current_heap_value;

        kernel_log("Heap was initialized.\n");
    }

    void reset()
    {
        current_heap_value = heap_start_value;
        kernel_log("Heap resetting has been completed.\n");
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
