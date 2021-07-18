;; print_string: prints a string stored in %ESI
print_string:
    pusha
.loop:
    lodsb
    cmp al, 0
    je .end
    call print_char
    jmp .loop
.end:
    popa
    ret

;; print_char: prints a char store in %AL
print_char:
    pusha
    push edi

    mov edi, 0xb8000
    imul edx, [cursorX], 2
    add edi, edx
    imul edx, [cursorY], 160
    add edi, edx

    cmp al, 0xA                 ; If we got 0xA
    je .Newline                 ; We must process new line
    
    mov ah, [printColor]
    stosw

    inc dword [cursorX]
    cmp dword [cursorX], 80
    jne .end

    inc dword [cursorY]
    mov dword [cursorX], 0
    jmp .end

.Newline:
    inc dword [cursorY]
    mov dword [cursorX], 0
.end:
    call update_scrolling
    call update_cursor

    pop edi
    popa
    ret

;; update_cursor: updates cursor position
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

;; clear_screen: clears the screen
clear_screen:
    pusha
    push edi

    mov edi, 0xb8000
    mov cx, 80 * 25
.loop:
    mov ah, [printColor]
    mov al, ' '
    stosw
    loop .loop

    pop edi
    popa
    ret

;; update_scrolling: updates all text on the screen to prevent its overflowing
update_scrolling:
    cmp dword [cursorY], 25
    jnge .end

    mov esi, 0xb8000+(80*2)
    mov edi, 0xb8000
    mov cx, (80*24)
.loop:
    movsw
    loop .loop

    mov edi, 0xb8000+(80*24*2)
    mov cx, 80
.loop2:
    mov ah, [printColor]
    mov al, ' '
    stosw
    loop .loop2

    mov dword [cursorY], 24     ; Move cursor to last line
.end:
    ret

cursorX: dd 0
cursorY: dd 0
printColor: db 0x07