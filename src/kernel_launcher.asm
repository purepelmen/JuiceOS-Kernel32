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

[bits 32]

extern kernel_main
extern clear_screen
extern print_string
extern ps2_waitKey

global start

start:
    cli
    mov esp, stack_top
    call verify_multiboot
    call check_cpuid

    call kernel_main
    hlt

verify_multiboot:
    cmp eax, 0x36d76289
    jne .fail
    ret
.fail:
    call clear_screen

    push no_multiboot_str
    call print_string
    add esp, 4
    call ps2_waitKey
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
    call ps2_waitKey
    jmp 0xffff0

no_multiboot_str: db "Kernel booted without multiboot! You cannot continue loading this OS.", 0xA
                  db "Press any key to reboot...", 0
no_cpuid_str:     db "CPUID is not supported on this PC! You cannot continue loading this OS.", 0xA
                  db "Press any key to reboot...", 0

%include "src/includes/cpuid.asm"
%include "src/includes/gdt.asm"
%include "src/includes/interrupts.asm"

section .bss
stack_bottom:
    resb 65536
stack_top: