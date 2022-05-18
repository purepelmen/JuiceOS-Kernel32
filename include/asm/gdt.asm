gdt_initialize:
    cli
    lgdt [gdt_descriptor]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.end
.end:
    ret

; ---------------------------------------------
; Global Description Table definition
; ---------------------------------------------

gdt_definition:
    dd 0x0              ; Null descriptor
    dd 0x0

.code_desc:
    dw 0xFFFF           ; Limit 0-15
    dw 0x0000           ; Base 0-15
    db 0x00             ; Base 16-23       
    db 10011010b        ; Access Flags
    db 11001111b        ; Limit 16-19 + Flags
    db 0x0              ; Base 24-31
.data_desc:
    dw 0xFFFF           ; Limit 0-15
    dw 0x0000           ; Base 0-15
    db 0x0              ; Base 16-23  
    db 10010010b        ; Access Flags
    db 11001111b        ; Limit 16-19 + Flags
    db 0x0              ; Base 24-31
.end:

gdt_descriptor:
    dw gdt_definition.end - gdt_definition - 1
    dd gdt_definition
