[bits 32]
[org 0x1000]

kernel_main:
    ; mov eax, 0x2f542f55
    ; mov [0xb8000], eax

    call clear_screen

lp:
    call ps2_keyboard_getkey

    cmp al, 5
    jne lp

    mov si, helloStr
    call print_string

    jmp lp

%include "src/includes/stdio.asm"
%include "src/includes/ps2.asm"

helloStr: db 'Hello! Kernel32 has been booted and runned successfully!', 0xA,\
             '-----------------------------------------------------------------', 0

times 4096-($-$$) db 0