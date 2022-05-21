section .multiboot_header
header_start:
    dd 0xe85250d6
    dd 0
    dd header_end - header_start

    dd 0x100000000 - (0xe85250d6 + 0 + (header_end-header_start))

    dw 0
    dw 0
    dd 8
header_end:

section .text
[bits 32]

extern kernel_main
global start

start:
    cli
    mov esp, stack_top
    call verify_multiboot

    call kernel_main
    jmp $

verify_multiboot:
    cmp eax, 0x36d76289
    jne .fail
    ret
.fail:
    add esp, 4
    jmp 0xffff0

section .bss
stack_bottom:
    resb 65536
stack_top: