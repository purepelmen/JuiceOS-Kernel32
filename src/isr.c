#include "inc/stdio.h"
#include "inc/ports.h"
#include "inc/isr.h"

isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs) {
    print_string("\nRecieved interrupt: 0x");
    print_hexb(regs.int_no);
    print_char('\n');

    if (interrupt_handlers[regs.int_no] != 0) {
        isr_t handler = interrupt_handlers[regs.int_no];
        handler(regs);
    }
}

void irq_handler(registers_t regs) {
    // Send an EOI (end of interrupt) signal to the PICs.
    // If this interrupt involved the slave.
    if(regs.int_no >= 40) {
        // Send reset signal to slave
        port_byte_out(0xA0, 0x20);
    }

    port_byte_out(0x20, 0x20);

    // Send reset signal to master. (As well as slave, if necessary).
    if(interrupt_handlers[regs.int_no] != 0) {
        isr_t handler = interrupt_handlers[regs.int_no];
        handler(regs);
    }
}