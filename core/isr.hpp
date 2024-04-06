#pragma once

#include "stdint.hpp"

#define IRQ_BASE 32

namespace kisr
{
    struct registers
    {
        uint32 ds;
        uint32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
        uint32 int_number, err_code;
        uint32 eip, cs, eflags, useresp, ss;
    };

    typedef struct registers regs_t;
    typedef void (*isr_handler_t)(regs_t);

    void register_handler(uint8 int_number, isr_handler_t handler);
}
