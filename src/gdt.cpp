#include "gdt.hpp"
#include "stdlib.hpp"

gdt_desc_t gdt_descriptors[3];

void gdt_set_gate(int desc_number, uint32 base, uint32 limit, bool readable_writable, bool is_executable, bool is_system_desc, uint8 privilege_level)
{
    if(privilege_level > 2)
    {
        RAISE_ERROR("Privilege level must be in range 0-2");
    }

    gdt_descriptors[desc_number].limit_lower = limit & 0xFFFF;
    gdt_descriptors[desc_number].base_lower = base & 0xFFFF;
    gdt_descriptors[desc_number].base_middle = (base >> 16) & 0xFF;

    uint8 access_byte = 0b10000000;
    access_byte &= readable_writable << 1;
    access_byte &= is_executable << 3;
    access_byte &= (!is_system_desc) << 4;
    access_byte &= privilege_level << 5;

    gdt_descriptors[desc_number].access_flags = access_byte;
    gdt_descriptors[desc_number].limit_high = (limit >> 16) & 0xFF & 0b00001111;
}
