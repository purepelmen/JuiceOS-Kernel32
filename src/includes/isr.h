#include "stdio.h"

typedef struct registers
{
   uint32_t ds;
   uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; 
   uint32_t int_no, err_code;
   uint32_t eip, cs, eflags, useresp, ss; 
} registers_t;

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs)
{
   print_string("recieved interrupt: ");
   print_hexb(regs.int_no);
   print_char('\n');
}