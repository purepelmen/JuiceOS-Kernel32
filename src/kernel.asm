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

    mov esi, inputBuffer
    mov edi, cmdCpuid
    call copmare_string
    cmp al, 0x01
    je cmd_cpuid

    mov esi, inputBuffer
    mov edi, cmdTest
    call copmare_string
    cmp al, 0x01
    je cmd_test

    mov esi, inputBuffer
    mov edi, cmdReboot
    call copmare_string
    cmp al, 0x01
    je cmd_reboot

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

cmd_cpuid:
    call print_cpu_info
    jmp input

cmd_test:
    mov si, testCmdHeaderStr
    call print_string

    call get_input

    cmp di, inputBuffer
    je .unknownDevice

    mov esi, inputBuffer
    mov edi, devicePS2Test
    call copmare_string
    cmp al, 0x01
    je .testPS2

    mov esi, inputBuffer
    mov edi, testDevicesList
    call copmare_string
    cmp al, 0x01
    je .devicesList

.unknownDevice:
    mov si, testDeviceNotFoundStr
    call print_string

    jmp input

.devicesList:
    mov si, devicesListStr
    call print_string
    jmp input

.testPS2:
    xor ax, ax
    mov al, 0xEE
    out 0x60, al
    xor ax, ax
    in al, 0x60
    cmp al, 0xEE
    je .TestComplete

    mov si, keyboardPs2TestFailureStr
    call print_string
    jmp input
.TestComplete:
    mov si, keyboardPs2TestSuccessStr
    call print_string
    jmp input

cmd_reboot:
    jmp 0xFFFF0

%include "src/includes/cpuid.asm"
%include "src/includes/stdio.asm"
%include "src/includes/ps2.asm"

testDeviceNotFoundStr: db 0xA, 'Typed device is not found! Type "test" and then "list" to get all devices you could test', 0xA, 0
keyboardPs2TestFailureStr: db 0xA, 'PS/2 keyboard not responded properly to echo-command. PS/2 Keyboard failed the test.', 0xA, 0
keyboardPs2TestSuccessStr: db 0xA, 'PS/2 keyboard responded to echo-command. PS/2 Keyboard passed the test.', 0xA, 0
testCmdHeaderStr: db 0xA, 'Please type here device you want to test (for instance - ps2): ', 0 
devicesListStr: db 0xA, 'Available devices to test: ps2 (PS/2 Keyboard).', 0xA, 0
commandNotFoundStr: db 0xA, 'Your typed command is not found.', 0xA, 0
systemInfoStr: db 0xA, 'JuiceOS Kernel32 v1.0', 0xA, 0
helloStr: db 0xA, 'Helloooo :)', 0xA, 0
prompt: db 'PC:>>', 0
newLine: db 0xA, 0

cmdSystem: db 'system', 0
cmdReboot: db 'reboot', 0
cmdHello: db 'hello', 0
cmdCpuid: db 'cpuid', 0
cmdTest: db 'test', 0

testDevicesList: db 'list', 0
devicePS2Test: db 'ps2', 0

times 4096-($-$$) db 0