#include "inc/stdio.h"
#include "inc/kheap.h"

// This defined in linker script. Points to end of the elf kernel file in memory.
extern uint32_t end;
// Address of the end of the elf kernel file in memory.
uint32_t placement_address = (uint32_t) &end;

uint32_t kmalloc_int(uint32_t size, int align, uint32_t* phys) {
    if(align == true && (placement_address & 0xFFFFF000)) {
        // Align the placment address
        placement_address &= 0xFFFFF000;
        placement_address += 0x1000;
    }

    if(phys) {
        *phys = placement_address;
    }

    uint32_t tmp = placement_address;
    placement_address += size;
    return tmp;
}

uint32_t kmalloc_a(uint32_t size) {
    return  kmalloc_int(size, 1, 0);
}

uint32_t kmalloc_p(uint32_t size, uint32_t *phys) {
    return kmalloc_int(size, 0, phys);
}

uint32_t kmalloc_ap(uint32_t size, uint32_t *phys) {
    return kmalloc_int(size, 1, phys);
}

uint32_t kmalloc(uint32_t size) {
    return kmalloc_int(size, 0, 0);
}