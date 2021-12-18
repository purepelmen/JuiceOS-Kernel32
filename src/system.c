#include "drivers/screen.h"
#include "isr/isr.h"
#include "kernel.h"
#include "system.h"
#include "stdio.h"

void zeroDeletionHandler(registers_t* regs)
{
    ShowError("Divided by zero", "unknown", "System Interrupt Handler", "Caused by 0x00 interrupt");
    PrintLog("Divided by zero error. Handled by 0x00 interrupt.\n");
    regs->eip += 1;
}

void RegisterSysHandlers(void)
{
    register_interrupt_handler(0, &zeroDeletionHandler);
}