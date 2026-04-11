#include "heap.h"
#include "stdlib.h"
#include "kernel.h"
#include "mmanager.h"

#include "free_llist_allocator.h"

/* The heap.h new[] template function uses it, because sometimes new[] requires the definition of this. */
extern "C" void __cxa_throw_bad_array_new_length()
{
    RAISE_ERROR("__cxa_throw_bad_array_new_length happened")
}

namespace kheap
{
    const size_t HEAP_SIZE = 12;
    static fll_allocator allocator;

    void setup()
    {
        void* heapMemory = kmmanager::alloc_pages(HEAP_SIZE);
        allocator.init(heapMemory, 4096 * HEAP_SIZE);

        kernel_log("Dynamic memory initialized (heap size: %d KB).\n", HEAP_SIZE * 4096 / 1024);
    }

    void* alloc(size_t size)
    {
        return allocator.alloc(size);
    }

    void free(void* ptr)
    {
        allocator.free(ptr);
    }

    void print_alloc_free_blocks()
    {
        allocator.dump();
    }

    uint32 get_heap_free()
    {
        return allocator.get_unused();
    }

    uint32 get_heap_size()
    {
        return allocator.get_heap_size();
    }
}
