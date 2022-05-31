#pragma once

#include "stdint.h"

namespace kidt
{
    struct idt_descriptor
    {
        uint16 offset_low;
        uint16 segment_selector;
        uint8 reserved_null;
        uint8 flags;
        uint16 offset_high;
    } __attribute__((packed));

    typedef struct idt_descriptor idt_desc_t;

    struct idt_register
    {
        uint16 table_size;
        uint32 table_offset;
    } __attribute__((packed));

    typedef struct idt_register idt_reg_t;

    void idt_init();
}
