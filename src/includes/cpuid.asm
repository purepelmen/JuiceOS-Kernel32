print_cpu_info:
    mov esi, cpuInfoHeaderStr
    call print_string

    xor eax, eax
    cpuid

    ; EBX
    mov al, bl
    call print_char
    mov al, bh 
    call print_char
    shr ebx, 16
    mov al, bl
    call print_char
    mov al, bh 
    call print_char

    ; EDX
    mov al, dl
    call print_char
    mov al, dh 
    call print_char
    shr edx, 16
    mov al, dl
    call print_char
    mov al, dh 
    call print_char

    ; ECX
    mov al, cl
    call print_char
    mov al, ch 
    call print_char
    shr ecx, 16
    mov al, cl
    call print_char
    mov al, ch 
    call print_char

    mov esi, cpuInfoHeader2Str
    call print_string

    ; Model printing

    mov eax, 0x80000002
    cpuid

    ; EAX
    call print_char
    mov al, ah 
    call print_char
    shr eax, 16
    call print_char
    mov al, ah 
    call print_char

    ; EBX
    mov al, bl
    call print_char
    mov al, bh 
    call print_char
    shr ebx, 16
    mov al, bl
    call print_char
    mov al, bh 
    call print_char

    ; ECX
    mov al, cl
    call print_char
    mov al, ch 
    call print_char
    shr ecx, 16
    mov al, cl
    call print_char
    mov al, ch 
    call print_char

    ; EDX
    mov al, dl
    call print_char
    mov al, dh 
    call print_char
    shr edx, 16
    mov al, dl
    call print_char
    mov al, dh 
    call print_char

    mov eax, 0x80000003
    cpuid

    ; EAX
    call print_char
    mov al, ah 
    call print_char
    shr eax, 16
    call print_char
    mov al, ah 
    call print_char

    ; EBX
    mov al, bl
    call print_char
    mov al, bh 
    call print_char
    shr ebx, 16
    mov al, bl
    call print_char
    mov al, bh 
    call print_char

    ; ECX
    mov al, cl
    call print_char
    mov al, ch 
    call print_char
    shr ecx, 16
    mov al, cl
    call print_char
    mov al, ch 
    call print_char

    ; EDX
    mov al, dl
    call print_char
    mov al, dh 
    call print_char
    shr edx, 16
    mov al, dl
    call print_char
    mov al, dh 
    call print_char

    mov eax, 0x80000004
    cpuid

    ; EAX
    call print_char
    mov al, ah 
    call print_char
    shr eax, 16
    call print_char
    mov al, ah 
    call print_char

    ; EBX
    mov al, bl
    call print_char
    mov al, bh 
    call print_char
    shr ebx, 16
    mov al, bl
    call print_char
    mov al, bh 
    call print_char

    ; ECX
    mov al, cl
    call print_char
    mov al, ch 
    call print_char
    shr ecx, 16
    mov al, cl
    call print_char
    mov al, ch 
    call print_char

    ; EDX
    mov al, dl
    call print_char
    mov al, dh 
    call print_char
    shr edx, 16
    mov al, dl
    call print_char
    mov al, dh 
    call print_char

    mov al, 0xA
    call print_char

    ret

;; Strings =====================================
cpuInfoHeaderStr: db 0xA, 'CPU Information ---------------------------------',\
                     0xA, 'CPU ID: ', 0
cpuInfoHeader2Str: db 0xA, 'CPU Model: ', 0