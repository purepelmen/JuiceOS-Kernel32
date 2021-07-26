[bits 32]

extern kmain
extern print_string

global start

start:
    cli
    mov esp, stack_top
    call kmain
    hlt

%include "src/includes/cpuid.asm"

section .bss
stack_bottom:
    resb 24028
stack_top: