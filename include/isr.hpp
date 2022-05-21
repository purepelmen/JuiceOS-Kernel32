#pragma once

#include "stdint.h"

namespace kisr
{
    struct isr_registers
    {
        uint32 ds;
        uint32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
        uint32 int_number, err_code;
        uint32 eip, cs, eflags, useresp, ss;
    };

    typedef struct isr_registers isr_regs_t;
    typedef void (*isr_handler_t)(isr_regs_t);

    void isr_register_handler(uint8 int_number, isr_handler_t handler);
}
