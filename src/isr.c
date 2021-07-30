#include "stdio.h"
#include "isr.h"

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs) {
    print_string("recieved interrupt: 0x");
    print_hexb(regs.int_no);
    print_char('\n');
}
