;; print: print given string
;; si - *string
print:
    mov ax, 0xb800
    mov es, ax

    imul di, [cursorY], 160
    imul ax, [cursorX], 2
    add di, ax

    mov ah, 0x17

.loop:
    lodsb
    cmp al, 0
    je .end
    cmp al, 0xA
    je .CRLF
    stosw
    inc word [cursorX]
    cmp word [cursorX], 80
    jne .loop
    inc word [cursorY]
    mov word [cursorX], 0
    jmp .loop

    .CRLF:
        mov word [cursorX], 0
        inc word [cursorY]
        jmp .loop
.end:
    ret

cursorX: dw 0
cursorY: dw 0