#include "ports.h"
#include "pit.h"
#include "../kernel.h"
#include "../isr.h"

#define PIT_FREQ_HZ 1193182

namespace ktimer
{
    // We get 1 kHZ, or 1 ms per tick.
    const int DIVISOR = PIT_FREQ_HZ / 1000;
    const float DEFAULT_FREQ = (float)PIT_FREQ_HZ / DIVISOR;
    const float MS_PER_TICK = 1000.0 / DEFAULT_FREQ;

    static volatile uint32 remainingTicks = 0;

    static void timer_handler(const kisr::regs_t& regs);
    
    void init()
    {
        port_write8(0x43, 0x36);
        port_write8(0x40, DIVISOR & 0xFF);
        port_write8(0x40, (DIVISOR >> 8) & 0xFF);

        kisr::register_handler(IRQ_BASE, timer_handler);
        kernel_print_log("Timer was initialized.\n");
    }

    void wait(uint32 ms)
    {
        remainingTicks = ms;
        while(remainingTicks > 0)
        {
            asm volatile("hlt");
        }
    }

    void timer_handler(const kisr::regs_t& regs)
    {
        if(remainingTicks != 0)
        {
            remainingTicks--;
        }
    }
}
