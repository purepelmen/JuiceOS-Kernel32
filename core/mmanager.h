#pragma once
#include "stdint.h"

namespace kmmanager
{
    /// @brief Initialize the memory manager.
    void init();

    void* alloc_pages(size_t amount);
    void free_pages(void* addr, size_t count);

    /// @brief Returns amount of bytes allocated dynamically by `alloc_pages()`.
    uint32 get_allocated();
    /// @brief Returns amount of bytes used by the kernel image (code+data), excluding any dynamic memory.
    uint32 get_kernel_memsize();
    uint32 get_stack_usage();

    void* get_location_ptr();
}
