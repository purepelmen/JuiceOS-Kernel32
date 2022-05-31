#pragma once

#include "stdint.h"

namespace kgdt
{
    struct gdt_descriptor
    {
        uint16 limit_lower;
        uint16 base_lower;
        uint8 base_middle;
        uint8 access_flags;
        uint8 limit_high: 4;
        uint8 other_flags: 4;
        uint8 base_high;
    } __attribute__((packed));

    typedef struct gdt_descriptor gdt_desc_t;

    struct gdt_register
    {
        uint16 table_size;
        uint32 table_offset;
    } __attribute__((packed));

    typedef struct gdt_register gdt_reg_t;

    void gdt_init();
}
