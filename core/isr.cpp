#include "drivers/screen.h"
#include "drivers/ports.h"
#include "kernel.h"
#include "isr.h"

#include "stdlib.h"
#include "console.h"

namespace kisr
{
    static isr_handler_t isr_handlers[256];

    static void pic_send_eoi(bool send_to_slave);
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
        pic_send_eoi(regs.int_number >= 40);
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

    static void pic_send_eoi(bool send_to_slave)
    {
        if(send_to_slave)
        {
            port_write8(0xA0, 0x20);
        }
        
        port_write8(0x20, 0x20);
    }
}
