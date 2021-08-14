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
extern clear_screen
extern print_string
extern ps2_readKey

global start

start:
    cli
    mov esp, stack_top
    call verify_multiboot
    call check_cpuid

    call kernel_main
    jmp $

verify_multiboot:
    cmp eax, 0x36d76289
    jne .fail
    ret
.fail:
    call clear_screen

    push no_multiboot_str
    call print_string
    add esp, 4
    call ps2_readKey
    jmp 0xffff0

check_cpuid:
    pushfd
	pop eax
	mov ecx, eax
	xor eax, 1 << 21
	push eax
	popfd
	pushfd
	pop eax
	push ecx
	popfd
	cmp eax, ecx
	je .no_cpuid
	ret
.no_cpuid:
	call clear_screen

    push no_cpuid_str
    call print_string
    add esp, 4
    call ps2_readKey
    jmp 0xffff0

;; GDT/IDT flush function definition
[global gdt_flush]
[global idt_flush]

gdt_flush:
    mov eax, [esp+4]  ; Get the pointer to the GDT, passed as a parameter.
    lgdt [eax]        ; Load the new GDT pointer

    mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax        ; Load all data segment selectors
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush   ; 0x08 is the offset to our code segment: Far jump!
.flush:
    ret

idt_flush:
    mov eax, [esp+4]  ; Get the pointer to the IDT, passed as a parameter. 
    lidt [eax]        ; Load the IDT pointer.
    sti               ; Enable interrupts
    ret


no_multiboot_str: db "Kernel booted without multiboot! You cannot continue loading this OS.", 0xA
                  db "Press any key to reboot...", 0
no_cpuid_str:     db "CPUID is not supported on this PC! You cannot continue loading this OS.", 0xA
                  db "Press any key to reboot...", 0

section .bss
stack_bottom:
    resb 65536
stack_top: