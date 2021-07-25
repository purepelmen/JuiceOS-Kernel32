[bits 32]
[org 0x1000]

kernel_main:
    ; mov eax, 0x2f542f55
    ; mov [0xb8000], eax

    call clear_screen
    jmp input_nbl

;; Start new input
input:
    mov si, newLine
    call print_string
input_nbl:
    mov si, prompt
    call print_string

    call get_input

;; Command handler
process_input:
    ;; If nothing was printed, skip it
    cmp di, inputBuffer
    je input

    mov esi, inputBuffer
    mov edi, cmdHello
    call copmare_string
    cmp al, 0x01
    je cmd_hello

    mov esi, inputBuffer
    mov edi, cmdSystem
    call copmare_string
    cmp al, 0x01
    je cmd_systemInfo

    mov si, commandNotFoundStr
    call print_string

    jmp input

cmd_hello:
    mov si, helloStr
    call print_string

    jmp input

cmd_systemInfo:
    mov si, systemInfoStr
    call print_string

    jmp input

%include "src/includes/cpuid.asm"
%include "src/includes/stdio.asm"
%include "src/includes/ps2.asm"

commandNotFoundStr: db 0xA, 'Your typed command is not found.', 0xA, 0
systemInfoStr: db 0xA, 'JuiceOS Kernel32 v1.0', 0xA, 0
helloStr: db 0xA, 'Helloooo :)', 0xA, 0
prompt: db 'PC:>>', 0
newLine: db 0xA, 0

cmdSystem: db 'system', 0
cmdHello: db 'hello', 0

times 4096-($-$$) db 0