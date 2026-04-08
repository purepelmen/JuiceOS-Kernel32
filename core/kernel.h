#pragma once
#include "string.h"
#include "stdlib.h"

#define KERNEL_VERSION "1.0.3"

struct mmap_entry
{
    unsigned long long addr;
    unsigned long long length;
    bool isAvailable;

    bool is_valid_addr_ptr() const { return addr <= 0xFFFFFFFF; }

    void* get_addr_ptr() const
    {
        kernel_assert(is_valid_addr_ptr(), "Trying to get addr ptr as void* while it's bigger than 0xFFFFFFFF.");
        return (void*)addr;
    }
};

class mmap_list
{
    const mmap_entry* m_begin;
    const mmap_entry* m_end;

public:
    mmap_list(const mmap_entry* begin, const mmap_entry* end) : m_begin(begin), m_end(end) {}

    const mmap_entry* begin() const { return m_begin; }
    const mmap_entry* end() const { return m_end; }
};

mmap_list kernel_get_mmap();

/// @brief Print new string to logs. 
void kernel_log(string str, ...);

bool kernel_render_logs(int pageIndex);
