#include "drivers/timer.hpp"
#include "isr.hpp"

namespace ktimer
{
    int remains_ticks = 0;

    static void timer_handler(kisr::regs_t regs);

    void init()
    {
        kisr::register_handler(IRQ_BASE, timer_handler);
    }

    void wait(int ticks)
    {
        remains_ticks = ticks;

        while(true)
        {
            asm volatile("hlt");

            if(remains_ticks == 0)
                break;
        }
    }

    static void timer_handler(kisr::regs_t regs)
    {
        if(remains_ticks == 0) return;
        remains_ticks--;
    }
}
