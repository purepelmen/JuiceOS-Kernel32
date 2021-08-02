// timer.c -- Initialises the PIT, and handles clock updates.

#include "inc/timer.h"
#include "inc/isr.h"
#include "inc/stdio.h"
#include "inc/kernel.h"
#include "inc/ports.h"

uint32 tick = 0;

static void timer_callback(registers_t regs) {
    tick++;
    print_string("Tick: ");
    print_hexdw(tick);
    print_string("\n");
}

void init_timer(uint32 freq) {
    // Firstly, register our timer callback.
    register_interrupt_handler(IRQ0, &timer_callback);

    // The value we send to the PIT is the value to divide it's input clock
    // (1193180 Hz) by, to get our required frequency. Important to note is
    // that the divisor must be small enough to fit into 16-bits.

    uint32 divisor = 1193180 / freq;

    // Send the command byte
    port_byte_out(0x43, 0x36);

    // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
    uint8 l = (uint8)(divisor & 0xFF);
    uint8 h = (uint8)( (divisor>>8) & 0xFF );

    // Send the frequency divisor.
    port_byte_out(0x40, l);
    port_byte_out(0x40, h);

    print_log("PIT was initialized.");
}