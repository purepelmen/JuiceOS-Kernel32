#pragma once

#include "stdint.h"

namespace kpaging
{
    struct directory_entry
    {
        uint8 present: 1;
        uint8 writable: 1;
        uint8 usermode_accessable: 1;
        uint8 write_through_caching: 1;
        uint8 caching_disabled: 1;
        uint8 was_accessed: 1;
        uint8 is_dirty: 1;
        uint8 four_mb_pages: 1;
        uint8 global: 1;
        uint8 ignored: 3;
        uint32 address: 20;
    } __attribute__((packed));

    typedef struct directory_entry dir_entry_t;

    void paging_init();
}
