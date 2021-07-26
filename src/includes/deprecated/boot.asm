[bits 16]
[org 0x7C00]

read_disk:
    mov ax, 0x0100              ; Memlocation where kernel must be loaded
    mov es, ax
    xor bx, bx

    mov ch, 0                   ; Cylinder to read
    mov cl, 2                   ; Starting sector to read
    mov dh, 0                   ; Head to read

    mov al, 8                   ; Sectors count to read
    mov ah, 0x02                ; AH 0x02 - Read operation
    int 0x13                    ; Call 0x13 interrupt (i/o operations with disk)

    jc read_error               ; If carry flag is set - print error

    cmp ah, 0                   ; IF read failed
    jne read_error              ; Print error

running_kernel:
    cli                         ; Clear interrupts

    xor ax, ax                  ; Clear ax
    mov ds, ax                  ; DS <- 0x0000
    lgdt [gdt_desc]             ; Load GDT

    mov eax, cr0                ; Get control reg 0
    or eax, 0x1                 ; Set first bit to 1 so we'll be in 32-bit
    mov cr0, eax                ; Apply changed bit. Since we're in 32-bit

    jmp CODE_SEG:pm_start       ; Set CS to code segment in gdt, and go to our label

[bits 32]
pm_start:
    mov ax, DATA_SEG            ; Set data and stack segments to data segment descriptor
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp                ; Set up stack pointer

    jmp 0x1000                  ; Jump to kernel

;; Prints a string
;; -------------------------------
print_string:
    pusha
    mov ah, 0x0e
print_string_loop:
    lodsb
    cmp al, 0
    je print_string_end
    int 0x10
    jmp print_string_loop
print_string_end:
    popa
    ret

;; Disk read error handlers
;; -------------------------------
read_error:
    mov si, readErr
    call print_string
    jmp $

%include "src/includes/gdt.asm"

readErr: db 'Kernel reading and loading failure!', 0

times 510-($-$$) db 0
dw 0xAA55