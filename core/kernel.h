#pragma once
#include "string.h"

#define KERNEL_VERSION "1.0.3"

struct multiboot_tag;

class mmap_entry
{
private:
    multiboot_tag* m_tag = nullptr;
    size_t m_entryIdx = 0;

public:
    mmap_entry() = default;
    mmap_entry(multiboot_tag* current, size_t entryIdx = 0) : m_tag(current), m_entryIdx(entryIdx) {}

    mmap_entry next();
    bool is_end() const;

    unsigned long long get_addr() const;
    size_t get_length() const;
    bool is_available() const;

    bool is_valid_addr_ptr() const;
    void* get_addr_ptr() const;
};

mmap_entry kernel_get_mmap();

/// @brief Print new string to logs. 
void kernel_log(string str, ...);

bool kernel_render_logs(int pageIndex);
