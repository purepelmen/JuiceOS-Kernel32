[global is_cpuid_supported]

; =====================================
; is_cpuid_supported: Checks for CPUID
; support. Returns bool.
; =====================================
is_cpuid_supported:
    pushfd                  ; Save EFALGS to restore when leaving this function.

    pushfd                  ; Store EFALGS...
    pop eax                 ; ... in EAX.
    mov ecx, eax            ; Will store here the original value for comparing.

    xor eax, 1 << 21        ; Invert the 21-th byte (ID flag).
    push eax                ; Store...
    popfd                   ; ... and apply it to EFLAGS.

    pushfd                  ; Read again...
    pop eax

    xor eax, ecx            ; Compare "changed" and original.
    jz .no_cpuid            ; They're equal? The bit hasn't changed, hence no CPUID.

    popfd
    mov eax, 1
    ret
.no_cpuid:
    popfd
    mov eax, 0
    ret
