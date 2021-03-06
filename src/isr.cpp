#include "drivers/screen.hpp"
#include "drivers/ports.hpp"
#include "stdlib.hpp"
#include "isr.hpp"

namespace kisr
{
    static isr_handler_t isr_handlers[256];

    static void pic_send_eoi(bool send_to_slave);
    static void isr_handle(isr_regs_t regs);

    void isr_register_handler(uint8 int_number, isr_handler_t handler)
    {
        isr_handlers[int_number] = handler;
    }

    extern "C" void isr_c_handler(isr_regs_t regs)
    {
        isr_handle(regs);
    }

    extern "C" void irq_c_handler(isr_regs_t regs)
    {
        pic_send_eoi(regs.int_number >= 40);
        isr_handle(regs);
    }

    static void isr_handle(isr_regs_t regs)
    {
        isr_handler_t handler = isr_handlers[regs.int_number];
        if(handler != 0)
        {
            handler(regs);
        }
    } 

    static void pic_send_eoi(bool send_to_slave)
    {
        if(send_to_slave)
        {
            port_byte_out(0xA0, 0x20);
        }
        
        port_byte_out(0x20, 0x20);
    }
}
