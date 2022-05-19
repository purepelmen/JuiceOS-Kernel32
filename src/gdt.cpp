#include "gdt.hpp"
#include "stdlib.hpp"

namespace kgdt
{
    const int GDT_DESCRIPTORS_COUNT = 3;

    gdt_desc_t gdt_descriptors[GDT_DESCRIPTORS_COUNT];
    gdt_reg_t gdtr;

    extern "C" void gdt_flush(uint32 pointer);

    void gdt_set_gate(int desc_number, uint32 base, uint32 limit, bool is_executable, uint8 privilege_level);
    void gdt_clear_gate(int desc_number);

    void gdt_init()
    {
        gdt_clear_gate(0);
        gdt_set_gate(1, 0x0, 0xFFFFF, true, 0);
        gdt_set_gate(2, 0x0, 0xFFFFF, false, 0);

        gdtr.table_size = sizeof(gdt_desc_t) * GDT_DESCRIPTORS_COUNT - 1;
        gdtr.table_offset = (uint32) &gdt_descriptors;

        gdt_flush((uint32) &gdtr);
    }

    void gdt_set_gate(int desc_number, uint32 base, uint32 limit, bool is_executable, uint8 privilege_level)
    {
        if(privilege_level > 2)
        {
            RAISE_ERROR("Privilege level must be in range 0-2");
        }

        gdt_descriptors[desc_number].limit_lower = limit & 0xFFFF;
        gdt_descriptors[desc_number].base_lower = base & 0xFFFF;
        gdt_descriptors[desc_number].base_middle = (base >> 16) & 0xFF;

        uint8 access_byte = 0b10010010;
        access_byte |= is_executable << 3;
        access_byte |= privilege_level << 5;

        gdt_descriptors[desc_number].access_flags = access_byte;
        gdt_descriptors[desc_number].limit_high = (limit >> 16) & 0xFF & 0b00001111;

        uint8 other_flags = 0b1100;
        gdt_descriptors[desc_number].other_flags = other_flags;
        gdt_descriptors[desc_number].base_high = (base >> 24) & 0xFF;
    }

    void gdt_clear_gate(int desc_number)
    {
        uint32 pointer = (uint32) &gdt_descriptors;
        pointer *= desc_number + 1;

        mem_fill((uint8*) pointer, 0, sizeof(gdt_desc_t));
    }
}
