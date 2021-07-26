[bits 32]

extern kernel_main
extern print_string

global start

start:
    cli
    mov esp, stack_top
    call kernel_main
    hlt

%include "src/includes/cpuid.asm"

section .bss
stack_bottom:
    resb 24028
stack_top: