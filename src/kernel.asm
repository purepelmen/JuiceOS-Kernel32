[bits 32]
[org 0x1000]

kernel_main:
    mov eax, 0x2f542f55
    mov [0xb8000], eax
    
    jmp $

times 4096-($-$$) db 0