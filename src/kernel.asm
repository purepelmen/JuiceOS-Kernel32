[bits 32]
[org 0x1000]

kernel_main:
    ; mov eax, 0x2f542f55
    ; mov [0xb8000], eax

    call clear_screen

    mov si, helloStr
    call print_string

loop:
    call ps2_waitKey
    call print_char
    jmp loop

%include "src/includes/cpuid.asm"
%include "src/includes/stdio.asm"
%include "src/includes/ps2.asm"

helloStr: db 'Hello! Kernel32 has been booted and runned successfully!', 0xA, 0

times 4096-($-$$) db 0