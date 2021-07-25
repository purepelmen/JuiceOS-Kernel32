;; print_hex_ascii - convert 4-bit number in al to ascii sybmol and prints it
;; #al - num, result - will be printed
print_hex_ascii:
    add al, 0x30
    cmp al, 0x39
    jle .End
    add al, 39
.End:
    call print_char
    ret

;; print_hexb - convert 8-bit number in al to ascii sybmol(will be up to 2 symbols) and prints it
;; #al - num, result - will be printed
print_hexb:
    push ax
    shr al, 4
    call print_hex_ascii
    pop ax
    push ax
    and al, 0x0F
    call print_hex_ascii
    pop ax
    ret

;; print_hexw - convert 16-bit number in ax to ascii sybmol(will be up to 4 symbols) and prints it
;; #ax - num, result - will be printed
print_hexw:
    push ax
    shr ax, 8
    call print_hexb
    pop ax
    push ax
    and ax, 0x00FF
    call print_hexb
    pop ax
    ret

;; print_hexdw - convert 32-bit number in eax to ascii sybmol(will be up to 8 symbols) and prints it
;; #eax - num, result - will be printed
print_hexdw:
    push eax
    shr eax, 16
    call print_hexw
    pop eax
    push eax
    and eax, 0x0000FFFF
    call print_hexw
    pop eax
    ret

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

    cmp al, 0x08                ; If we got 0x08
    je .Backspace               ; We must process new line
    
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
    jmp .end

.Backspace:
    cmp dword [cursorX], 0
    je .end

    dec dword [cursorX]

    mov edi, 0xb8000
    imul edx, [cursorX], 2
    add edi, edx
    imul edx, [cursorY], 160
    add edi, edx

    mov ah, [printColor]
    mov al, ' '
    stosw
.end:
    call update_scrolling
    call update_cursor

    pop edi
    popa
    ret

;; get_string_length: get string length
;; esi - *string pointer | Return: ecx - chars count
get_string_length:
    xor ecx, ecx
    push esi
.loop:
    lodsb
    cmp al, 0
    je .end
    inc ecx
    jmp .loop
.end:
    pop esi
    ret

;; compare_string_length: compare two string lengths
;; esi - *first string pointer, edi - *second string pointer
;; Return: al - 0x01 = true (Length are equal) | al - 0x00 = false
compare_string_length:
    push esi
    call get_string_length
    mov edx, ecx

    mov esi, edi
    call get_string_length
    pop esi

    cmp ecx, edx
    je .Equal
    jmp .NEqual

.Equal:
    mov al, 0x01
    ret
.NEqual:
    mov al, 0x00
    ret

;; compare_string: compare two strings
;; esi - *first string pointer, edi - *second string pointer
;; Return: al - 0x01 = true (strings are equal) | al - 0x00 = false
copmare_string:
    ;; Compare length
    call compare_string_length
    cmp al, 0x01
    jne .NEqual

    ;; Loop comparison
    cld
    rep cmpsb
    jne .NEqual
    jmp .Equal
.NEqual:
    mov al, 0x00
    ret
.Equal:
    mov al, 0x01
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

    call update_cursor

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

;; TODO: Make input procedure
get_input:
    call reset_inputBuffer
    mov edi, inputBuffer
.inputLoop:
    call ps2_waitKey

    cmp al, 0xA
    je .end

    cmp al, 0x08
    je .HandleBackSpace

    cmp edi, inputBuffer + 58
    jge .inputLoop

    call print_char
    stosb

    jmp .inputLoop

.HandleBackSpace:
    ;; If we at start of input, we shouldn't handle backspace
    cmp edi, inputBuffer
    je .inputLoop

    call print_char
    dec edi
    jmp .inputLoop

.end:
    ret

;; reset_inputBuffer: fill input buffer by zeros to reset it
reset_inputBuffer:
    pusha
    mov cx, 60
    mov edi, inputBuffer
.loop:
    mov al, 0
    stosb
    loop .loop
    popa
    ret

cursorX: dd 0
cursorY: dd 0
printColor: db 0x07

inputBuffer: times 60 db 0