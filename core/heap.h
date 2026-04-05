#pragma once
#include "stdlib.h"

/* Pre-declaration of the placement new. Without this, the placement new won't work. */
inline void* operator new(size_t size, void* p) noexcept { return p; }
inline void* operator new[](size_t size, void* p) noexcept { return p; }

namespace kheap
{
    /// @brief Initialize the dynamic memory system.
    void init();
    /// @brief Perform memory reset. It discards all allocated data with `sbrk()`.
    void reset();

    /// @brief Expands dynamic memory controlled by the kernel by given size and gives ptr to the allocated space.
    void* sbrk(size_t size);
    /// @brief Same as `sbrk()` but with 4KB alignment.
    void* sbrk_pgaligned(size_t size);

    /// @brief Allocate a memory block of given size from the heap. Must be freed with `free()`.
    void* alloc(size_t size);
    /// @brief Reclaim memory allocated by `alloc()`.
    void free(void* ptr);

    void print_alloc_free_blocks();

    /// @brief Returns amount of bytes allocated dynamically by `sbrk()`.
    uint32 get_allocated();
    /// @brief Returns amount of bytes used by the kernel image (code+data), excluding any dynamic memory.
    uint32 get_kernel_memsize();
    uint32 get_heap_free();
    uint32 get_heap_size();
    uint32 get_stack_usage();

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
        constexpr size_t HeaderSize = sizeof(size_t);

        void* allocPtr = alloc(HeaderSize + sizeof(T) * count);
        ((size_t*)allocPtr)[0] = count;

        return new ((uint8*)allocPtr + HeaderSize) T[count];
    }

    template<typename T>
    void destroy(T* obj)
    {
        if constexpr (!__has_trivial_destructor(T))
        {
            obj->~T();
        }

        free(obj);
    }

    template<typename T>
    void destroy_array(T* elements)
    {
        constexpr size_t HeaderSize = sizeof(size_t);

        T* arrayPtr = (T*)((uint8*)elements - HeaderSize);
        if constexpr (!__has_trivial_destructor(T))
        {
            size_t count = ((size_t*)arrayPtr)[0];
            for (size_t i = count - 1; i >= 0; --i)
            {
                elements[i].~T();
            }
        }

        free(arrayPtr);
    }
}
