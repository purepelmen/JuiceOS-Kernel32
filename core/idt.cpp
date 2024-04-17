#include "drivers/ports.h"
#include "stdlib.h"
#include "kernel.h"
#include "idt.h"

namespace kidt
{
    const int IDT_DESCRIPTORS_COUNT = 256;

    static idt_desc_t idt_descriptors[256];
    static idt_reg_t idtr;

    extern "C" void idt_flush(idt_reg_t* pointer);

    extern "C" uint32 isr_handlers_offset;
    extern "C" uint8 isr_handler_size;

    static void idt_set_gate(int desc_number, uint32 handler_offset, uint16 segment_selector, uint8 flags);
    static void pic_set_irq_offsets(uint8 master_offset, uint8 slave_offset);

    void idt_init()
    {
        pic_set_irq_offsets(32, 40);
        
        for(int i = 0; i < 48; i++)
        {
            uint32 handler_offset = isr_handlers_offset + isr_handler_size * i;
            idt_set_gate(i, handler_offset, 0x08, 0x8E);
        }

        idtr.table_size = sizeof(idt_desc_t) * IDT_DESCRIPTORS_COUNT - 1;
        idtr.table_offset = idt_descriptors;

        idt_flush(&idtr);
        kernel_print_log("IDT initialized.\n");
    }

    static void idt_set_gate(int desc_number, uint32 handler_offset, uint16 segment_selector, uint8 flags)
    {
        idt_descriptors[desc_number].offset_low = handler_offset & 0xFFFF;
        idt_descriptors[desc_number].segment_selector = segment_selector;
        idt_descriptors[desc_number].reserved_null = 0;
        idt_descriptors[desc_number].flags = flags;
        idt_descriptors[desc_number].offset_high = (handler_offset >> 16) & 0xFFFF;
    }

    static void pic_set_irq_offsets(uint8 master_offset, uint8 slave_offset)
    {
        port_byte_out(0x20, 0x11);
        port_byte_out(0xA0, 0x11);

        port_byte_out(0x21, master_offset);
        port_byte_out(0xA1, slave_offset);

        port_byte_out(0x21, 0x04);
        port_byte_out(0xA1, 0x02);

        port_byte_out(0x21, 0x01);
        port_byte_out(0xA1, 0x01);

        port_byte_out(0x21, 0x0);
        port_byte_out(0xA1, 0x0);
    }
}
