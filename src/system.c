#include "inc/kernel.h"
#include "inc/system.h"
#include "inc/stdio.h"
#include "inc/isr.h"

void zeroDeletionHandler(registers_t* regs) {
    showError("Divided by zero", "unknown", "System Interrupt Handler", "Caused by 0x00 interrupt");
    print_log("Divided by zero error. Handled by 0x00 interrupt.\n");
    regs->eip += 1;
}

void registerSystemHandlers(void) {
    register_interrupt_handler(0, &zeroDeletionHandler);
}