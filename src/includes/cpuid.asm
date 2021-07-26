global get_cpuid_info
global get_cpu_model_info

get_cpuid_info:
    push ebp
    mov ebp, esp

    pushad

    mov edi, cpuInfoStr

    xor eax, eax
    cpuid

    ; EBX
    mov al, bl
    stosb
    mov al, bh 
    stosb
    shr ebx, 16
    mov al, bl
    stosb
    mov al, bh 
    stosb

    ; EDX
    mov al, dl
    stosb
    mov al, dh 
    stosb
    shr edx, 16
    mov al, dl
    stosb
    mov al, dh 
    stosb

    ; ECX
    mov al, cl
    stosb
    mov al, ch 
    stosb
    shr ecx, 16
    mov al, cl
    stosb
    mov al, ch 
    stosb

    popad

    mov eax, cpuInfoStr

    pop ebp
    ret

get_cpu_model_info:
    push ebp
    mov ebp, esp

    pushad

    mov edi, cpuModelInfoStr

    mov eax, 0x80000002
    cpuid

    ; EAX
    stosb
    mov al, ah 
    stosb
    shr eax, 16
    stosb
    mov al, ah 
    stosb

    ; EBX
    mov al, bl
    stosb
    mov al, bh 
    stosb
    shr ebx, 16
    mov al, bl
    stosb
    mov al, bh 
    stosb

    ; ECX
    mov al, cl
    stosb
    mov al, ch 
    stosb
    shr ecx, 16
    mov al, cl
    stosb
    mov al, ch 
    stosb

    ; EDX
    mov al, dl
    stosb
    mov al, dh 
    stosb
    shr edx, 16
    mov al, dl
    stosb
    mov al, dh 
    stosb

    mov eax, 0x80000003
    cpuid

    ; EAX
    stosb
    mov al, ah 
    stosb
    shr eax, 16
    stosb
    mov al, ah 
    stosb

    ; EBX
    mov al, bl
    stosb
    mov al, bh 
    stosb
    shr ebx, 16
    mov al, bl
    stosb
    mov al, bh 
    stosb

    ; ECX
    mov al, cl
    stosb
    mov al, ch 
    stosb
    shr ecx, 16
    mov al, cl
    stosb
    mov al, ch 
    stosb

    ; EDX
    mov al, dl
    stosb
    mov al, dh 
    stosb
    shr edx, 16
    mov al, dl
    stosb
    mov al, dh 
    stosb

    mov eax, 0x80000004
    cpuid

    ; EAX
    stosb
    mov al, ah 
    stosb
    shr eax, 16
    stosb
    mov al, ah 
    stosb

    ; EBX
    mov al, bl
    stosb
    mov al, bh 
    stosb
    shr ebx, 16
    mov al, bl
    stosb
    mov al, bh 
    stosb

    ; ECX
    mov al, cl
    stosb
    mov al, ch 
    stosb
    shr ecx, 16
    mov al, cl
    stosb
    mov al, ch 
    stosb

    ; EDX
    mov al, dl
    stosb
    mov al, dh 
    stosb
    shr edx, 16
    mov al, dl
    stosb
    mov al, dh 
    stosb

    mov al, 0xA
    stosb

    popad

    mov eax, cpuModelInfoStr

    pop ebp
    ret

cpuInfoStr: times 16 db 0
cpuModelInfoStr: times 100 db 0