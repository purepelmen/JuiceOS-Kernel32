#include "stdlib.h"
#include "string.h"
#include "kernel.h"
#include "heap.h"

#include "free_llist_allocator.h"

extern uint32 end;
extern uint32 stack_base_addr;

/* The heap.h new[] template function uses it, because sometimes new[] requires the definition of this. */
extern "C" void __cxa_throw_bad_array_new_length()
{
    RAISE_ERROR("__cxa_throw_bad_array_new_length happened")
}

namespace kheap
{
    const size_t HEAP_SIZE = 1024 * 64;

    static uint8* dynmemStart;
    static uint8* dynmemPtr;

    void init()
    {
        // heap_start_value = (uint8*)&end + (1024 * 64);
        dynmemStart = (uint8*)&end;
        dynmemPtr = dynmemStart;

        void* heapMemory = sbrk(HEAP_SIZE);
        heap_free_llist_init(heapMemory, HEAP_SIZE);

        kernel_log("Dynamic memory initialized (heap size: %d KB).\n", HEAP_SIZE / 1024);
    }

    void reset()
    {
        dynmemPtr = dynmemStart;

        void* heapMemory = sbrk(HEAP_SIZE);
        heap_free_llist_init(heapMemory, HEAP_SIZE);

        kernel_log("Heap resetting has been completed. This will break references.\n");
    }

    void* sbrk(size_t size)
    {
        uint8* tmp = dynmemPtr;
        dynmemPtr += size;

        return tmp;
    }

    void* sbrk_pgaligned(size_t size)
    {
        uint32 currentPtr = (uint32)dynmemPtr;
        if(currentPtr & 0xFFFFF000)
        {
            currentPtr &= 0xFFFFF000;
            currentPtr += 0x1000;
        }
        dynmemPtr = (uint8*) currentPtr;

        return sbrk(size);
    }

    void* alloc(size_t size)
    {
        return heap_free_llist_alloc(size);
    }

    void free(void* ptr)
    {
        heap_free_llist_free(ptr);
    }

    void print_alloc_free_blocks()
    {
        heap_free_llist_dump_free_blocks();
    }

    uint32 get_allocated()
    {
        return (uint32) (dynmemPtr - dynmemStart);
    }

    uint32 get_kernel_memsize()
    {
        return (uint32) (dynmemStart - 0x100000);
    }

    uint32 get_heap_free()
    {
        return heap_free_llist_get_unused();
    }

    uint32 get_heap_size()
    {
        return HEAP_SIZE;
    }

    uint32 get_stack_usage()
    {
        uint32 esp;
        asm volatile ("movl %%esp, %0" : "=r" (esp));

        return stack_base_addr - esp;
    }

    void *get_location_ptr()
    {
        return dynmemStart;
    }
}
