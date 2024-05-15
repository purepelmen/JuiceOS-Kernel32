#include "ports.h"
#include "pit.h"
#include "../kernel.h"
#include "../isr.h"

namespace ktimer
{
    int remains_ms = 0;

    static void timer_handler(kisr::regs_t regs);

    void init()
    {
        // We need 1 KHz (1000 ticks per second, 1 tick = 1 ms)
        uint32 divisor = 1193182 / 1000;

        port_write8(0x43, 0x36);
        port_write8(0x40, divisor & 0xFF);
        port_write8(0x40, (divisor >> 8) & 0xFF);

        kisr::register_handler(IRQ_BASE, timer_handler);
        kernel_print_log("Timer was initialized.\n");
    }

    void wait(int ms)
    {
        remains_ms = ms;

        while(remains_ms != 0)
        {
            asm volatile("hlt");
        }
    }

    static void timer_handler(kisr::regs_t regs)
    {
        if(remains_ms == 0) return;
        remains_ms--;
    }
}
