#include "drivers/ports.h"
#include "drivers/pic.h"

#include "idt.h"
#include "stdlib.h"
#include "kernel.h"

namespace kidt
{
    const int IDT_DESCRIPTORS_COUNT = 256;

    static idt_desc_t idt_descriptors[256];
    static idt_reg_t idtr;

    extern "C" void idt_flush(idt_reg_t* pointer);

    extern "C" uint32 isr_handlers_offset;
    extern "C" uint8 isr_handler_size;

    static void idt_set_gate(int desc_number, uint32 handler_offset, uint16 segment_selector, uint8 gateType, uint8 privilegeLevel);

    void idt_init()
    {
        pic_init(32, 40);
        
        for(int i = 0; i < 48; i++)
        {
            uint32 handler_offset = isr_handlers_offset + isr_handler_size * i;
            idt_set_gate(i, handler_offset, 0x08, IDT_GATE_TYPE_INTERRUPT, 0);
        }

        idtr.table_size = sizeof(idt_desc_t) * IDT_DESCRIPTORS_COUNT - 1;
        idtr.table_offset = idt_descriptors;

        idt_flush(&idtr);
        kernel_log("IDT initialized.\n");
    }

    static void idt_set_gate(int desc_number, uint32 handler_offset, uint16 segment_selector, uint8 gateType, uint8 privilegeLevel)
    {
        if (privilegeLevel > 3)
        {
            RAISE_ERROR_D("idt_set_gate() incorrect DPL", "DPL must be in range 0-3, but '%d' passed.", privilegeLevel);
        }

        idt_descriptors[desc_number].offset_low = handler_offset & 0xFFFF;
        idt_descriptors[desc_number].segment_selector = segment_selector;
        idt_descriptors[desc_number].reserved_null = 0;
        idt_descriptors[desc_number].flags.is_present = 1;
        idt_descriptors[desc_number].flags.gate_type = gateType;
        idt_descriptors[desc_number].flags.dpl = privilegeLevel;
        idt_descriptors[desc_number].flags.storage_segment = 0;
        idt_descriptors[desc_number].offset_high = (handler_offset >> 16) & 0xFFFF;
    }
}
