#pragma once

#include "stdint.h"

#define IDT_TASK_GATE_TYPE          0x05
#define IDT_INTERRUPT_GATE_TYPE     0x0E
#define IDT_TRAP_GATE_TYPE          0x0F

namespace kidt
{
    struct idt_descriptor
    {
        uint16 offset_low;
        uint16 segment_selector;
        uint8 reserved_null;

        struct
        {
            uint8 gate_type : 4;
            uint8 storage_segment : 1;
            uint8 dpl : 2;
            uint8 is_present : 1;
        } __attribute__((packed)) flags;

        uint16 offset_high;
    } __attribute__((packed));

    typedef struct idt_descriptor idt_desc_t;

    struct idt_register
    {
        uint16 table_size;
        idt_desc_t* table_offset;
    } __attribute__((packed));

    typedef struct idt_register idt_reg_t;

    void idt_init();
}
