; =====================================
; paging_enable: enables paging
; =====================================
[global paging_enable]
paging_enable:
    mov eax, cr4                ; Enable PSE (for 4-Mb pages)
    or eax, 0x00000010
    mov cr4, eax

    mov eax, [esp+4]            ; Load page directory from argument
    mov cr3, eax

    mov eax, cr0                ; Enable paging
    or eax, 0x80000001
    mov cr0, eax

    ret
