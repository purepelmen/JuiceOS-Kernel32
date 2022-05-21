; =====================================
; idt_flush: sets pointer to GDTR
; =====================================
[global idt_flush]
idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    sti
    ret

; =====================================
; Interrupt Service Routine handlers
; =====================================

ISR_HANDLER_SIZE_CONST equ 14

%macro ISR_NOERRCODE 1
isr_handler_%1:
    cli
    push byte 0
    push byte %1
    jmp isr_common_stub

    ; Fill ISR handler to fit size with other handler types
    times ISR_HANDLER_SIZE_CONST - ($-isr_handler_%1) db 0
%endmacro

%macro ISR_ERRCODE 1
isr_handler_%1:
    cli
    push byte %1
    jmp isr_common_stub

    ; Fill ISR handler to fit size with other handler types
    times ISR_HANDLER_SIZE_CONST - ($-isr_handler_%1) db 0
%endmacro

%macro IRQ 1
irq_handler_%1:
    cli
    push byte 0
    push byte %1
    jmp irq_common_stub

    ; Fill IRQ handler to fit size with other handler types
    times ISR_HANDLER_SIZE_CONST - ($-irq_handler_%1) db 0
%endmacro

[extern isr_c_handler]
isr_common_stub:
    pusha

    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_c_handler

    pop ebx
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    popa
    add esp, 8
    sti
    iret

[extern irq_c_handler]
irq_common_stub:
    pusha
    
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call irq_c_handler

    pop ebx
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    popa
    add esp, 8
    sti
    iret

[global isr_handlers_offset]
[global isr_handler_size]

isr_handler_size: db ISR_HANDLER_SIZE_CONST
isr_handlers_offset: dd isr_handlers_start

isr_handlers_start:
    ISR_NOERRCODE 0
    ISR_NOERRCODE 1
    ISR_NOERRCODE 2
    ISR_NOERRCODE 3
    ISR_NOERRCODE 4
    ISR_NOERRCODE 5
    ISR_NOERRCODE 6
    ISR_NOERRCODE 7
    ISR_ERRCODE   8
    ISR_NOERRCODE 9
    ISR_ERRCODE   10
    ISR_ERRCODE   11
    ISR_ERRCODE   12
    ISR_ERRCODE   13
    ISR_ERRCODE   14
    ISR_NOERRCODE 15
    ISR_NOERRCODE 16
    ISR_NOERRCODE 17
    ISR_NOERRCODE 18
    ISR_NOERRCODE 19
    ISR_NOERRCODE 20
    ISR_NOERRCODE 21
    ISR_NOERRCODE 22
    ISR_NOERRCODE 23
    ISR_NOERRCODE 24
    ISR_NOERRCODE 25
    ISR_NOERRCODE 26
    ISR_NOERRCODE 27
    ISR_NOERRCODE 28
    ISR_NOERRCODE 29
    ISR_NOERRCODE 30
    ISR_NOERRCODE 31

    IRQ           0     ; 32
    IRQ           1     ; 33
    IRQ           2     ; 34
    IRQ           3     ; 35
    IRQ           4     ; 36
    IRQ           5     ; 37
    IRQ           6     ; 38
    IRQ           7     ; 39
    IRQ           8     ; 40
    IRQ           9     ; 41
    IRQ           10    ; 42
    IRQ           11    ; 43
    IRQ           12    ; 44
    IRQ           13    ; 45
    IRQ           14    ; 46
    IRQ           15    ; 47
