global cpuid_get_id
global cpuid_get_model

cpuid_get_id:
    push ebp
    mov ebp, esp
    pushad
    
    mov edi, cpuInfoStr
    xor eax, eax
    cpuid

    ; EBX
    mov eax, ebx
    stosd
    ; EDX
    mov eax, edx
    stosd
    ; ECX
    mov eax, ecx
    stosd

    popad
    mov eax, cpuInfoStr
    pop ebp
    ret

cpuid_get_model:
    push ebp
    mov ebp, esp
    pushad

    mov edi, cpuModelInfoStr
    mov eax, 0x80000002
    cpuid

    ; EAX
    stosd
    ; EBX
    mov eax, ebx
    stosd
    ; ECX
    mov eax, ecx
    stosd
    ; EDX
    mov eax, edx
    stosd

    mov eax, 0x80000003
    cpuid

    ; EAX
    stosd
    ; EBX
    mov eax, ebx
    stosd
    ; ECX
    mov eax, ecx
    stosd
    ; EDX
    mov eax, edx
    stosd

    mov eax, 0x80000004
    cpuid

    ; EAX
    stosd
    ; EBX
    mov eax, ebx
    stosd
    ; ECX
    mov eax, ecx
    stosd
    ; EDX
    mov eax, edx
    stosd

    popad
    mov eax, cpuModelInfoStr
    pop ebp
    ret

cpuInfoStr: times 14 db 0
cpuModelInfoStr: times 50 db 0