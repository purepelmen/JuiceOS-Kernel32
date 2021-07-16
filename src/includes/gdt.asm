GDT_start:
    ;; Offset 0x0
    dq 0            ; NULL Descriptor

    ;; Offset 0x8
gdt_code:
    dw 0xFFFF       ; Segment limit 2 bytes
    dw 0x0000       ; Segment base 1-2 bytes
    db 0x00         ; Segment bse 2-1 byte
    db 10011010b    ; Access byte
    db 11001111b    ; Flags
    db 0x00         ; Segment base 3-1 byte

    ;; Offset 0x10
gdt_data:
    dw 0xFFFF       ; Segment limit 2 bytes
    dw 0x0000       ; Segment base 1-2 bytes
    db 0x00         ; Segment bse 2-1 byte
    db 10010010b    ; Access byte
    db 11001111b    ; Flags
    db 0x00         ; Segment base 3-1 byte
GDT_end:

gdt_desc:
    dw GDT_end - GDT_start - 1
    dd GDT_start

CODE_SEG equ gdt_code - GDT_start       ; 0x8
DATA_SEG equ gdt_data - GDT_start       ; 0x10