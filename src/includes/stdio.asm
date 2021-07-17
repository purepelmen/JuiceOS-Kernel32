print_string:
    lodsb
    cmp al, 0
    je .end
    call print_char
    jmp print_string
.end:
    ret

print_char:
    mov edi, 0xb8000
    imul edx, [cursorX], 2
    add edi, edx
    imul edx, [cursorY], 160
    add edi, edx

    cmp al, 0xA                 ; If we got 0xA
    je .Newline                 ; We must process new line
    
    mov ah, [printColor]
    stosw

    inc word [cursorX]
    cmp word [cursorX], 80
    jne .end

    inc word [cursorY]
    mov word [cursorX], 0
    jmp .end

.Newline:
    inc word [cursorY]
    mov word [cursorX], 0
.end:
    call update_cursor
    ret

update_cursor:
    push eax
    push ebx
    push edx

    mov ebx, [cursorX]
    mov eax, [cursorY]

    mov dl, 80
    mul dl
    add bx, ax

    mov dx, 0x03D4
    mov al, 0x0F
    out dx, al

    inc dl
    mov al, bl
    out dx, al

    dec dl
    mov al, 0x0E
    out dx, al

    inc dl
    mov al, bh
    out dx, al

    pop edx
    pop ebx
    pop eax
    ret

clear_screen:
    mov edi, 0xb8000
    mov cx, 80 * 25
.loop:
    mov ah, [printColor]
    mov al, ' '
    stosw
    loop .loop

    ret

cursorX: dd 0
cursorY: dd 0
printColor: db 0x07