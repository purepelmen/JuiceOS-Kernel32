#include "drivers/screen.h"
#include "drivers/ports.h"
#include "drivers/pic.h"
#include "kernel.h"
#include "isr.h"

#include "stdlib.h"
#include "console.h"

namespace kisr
{
    static isr_handler_t isr_handlers[256];

    static void handle_isr(const regs_t& regs);

    void register_handler(uint8 int_number, isr_handler_t handler)
    {
        if(isr_handlers[int_number] != 0)
        {
            kernel_log("WARNING: An ISR handler was replaced.\n");
        }

        isr_handlers[int_number] = handler;
    }

    extern "C" void isr_c_handler(regs_t regs)
    {
        handle_isr(regs);
    }

    extern "C" void irq_c_handler(regs_t regs)
    {
        uint32 irqNumber = regs.int_number - 32;

        pic_send_eoi(irqNumber >= 8);
        if (irqNumber == 7 || irqNumber == 15)
        {
            uint16 isr = pic_get_isr();
            if (isr & (1 << irqNumber))
            {
                // This is spurious interrupt.
                // If from slave, send EOI to the master PIC only.
                if (irqNumber >= 8)
                    pic_send_eoi(false);

                return;
            }
        }

        handle_isr(regs);
    }

    static void handle_isr(const regs_t& regs)
    {
        isr_handler_t handler = isr_handlers[regs.int_number];
        if(handler != 0)
        {
            handler(regs);
            return;
        }

        if(regs.int_number < 32)
        {
            kconsole::clear();
            kconsole::printf("Interrupt: #%d\n", regs.int_number);

            RAISE_ERROR("Unhandled exception, trap or fault");
        }
    }
}
