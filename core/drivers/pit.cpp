#include "ports.h"
#include "pit.h"
#include "../kernel.h"
#include "../isr.h"

#define PIT_FREQ_HZ 1193181.666

namespace ktimer
{
    // We get 100 HZ, or 10 ms per tick.
    const int DIVISOR = (PIT_FREQ_HZ / 100.0);
    const int DEFAULT_FREQ = (float)PIT_FREQ_HZ / DIVISOR;
    const int MS_PER_TICK = 1000.0 / DEFAULT_FREQ;

    // May be because of too high frequency. Let's ensure we won't divide by zero. 
    static_assert(MS_PER_TICK > 0);

    static volatile unsigned long long ticks = 0ULL;

    static void timer_handler(const kisr::regs_t& regs);
    
    void init()
    {
        port_write8(0x43, 0x36);
        port_write8(0x40, DIVISOR & 0xFF);
        port_write8(0x40, (DIVISOR >> 8) & 0xFF);

        kisr::register_handler(IRQ_BASE, timer_handler);
        kernel_log("Timer was initialized.\n");
    }

    void wait(uint32 ms)
    {
        auto goal = ticks + ms / MS_PER_TICK;
        while(ticks < goal)
        {
            asm volatile("hlt");
        }
    }

    void timer_handler(const kisr::regs_t& regs)
    {
        ticks++;
    }
}
