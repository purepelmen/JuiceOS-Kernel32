#pragma once
#include "stdlib.h"

/* Pre-declaration of the placement new. Without this, the placement new won't work. */
inline void* operator new(size_t size, void* p) noexcept { return p; }
inline void* operator new[](size_t size, void* p) noexcept { return p; }

namespace kheap
{
    /* Initialize the heap system. */
    void init();

    /* Reset the heap. */
    void reset();

    /* Allocate a block of memory. */
    void* alloc(uint32 size);

    /* Allocate a 4K aligned block of memory. */
    void* alloc_pg_aligned(uint32 size);

    uint32 get_allocated_size();

    uint32 get_system_mem_size();

    void* get_location_ptr();

    // Allocate a block of memory and cast the pointer to the type.
    template<typename T>
    inline T* alloc_casted(uint32 size)
    {
        return (T*) alloc(size);
    }

    // A working alternative to the new operator that uses the kernel heap allocator.
    template<typename T, typename... Args>
    T* create_new(Args&& ...args)
    {
        void* allocPtr = alloc(sizeof(T));
        return new (allocPtr) T(args...);
    }

    // A working alternative to the new[] operator that uses the kernel heap allocator.
    template<typename T>
    T* create_new_array(size_t count)
    {
        void* allocPtr = alloc(sizeof(T) * count);
        return new (allocPtr) T[count];
    }
}
