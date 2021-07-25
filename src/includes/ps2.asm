ps2_keyboard_getkey:
    in al, 0x64
    test al, 0011b              ; Check if it is not pressed
    jz ps2_keyboard_getkey      ; Loop 

    in al, 0x60                 ; 60h port - returns the latest pressed key
    ret

ps2_waitKey:
    call ps2_keyboard_getkey

    cmp al, 0x2A                ; LShift pressDown
    je .LShiftDown

    cmp al, 0xAA                ; LShift pressUp
    je .LShiftUp

    cmp al, 0x1D                ; LCtrl pressDown
    je .LCtrlDown

    cmp al, 0x9D                ; LCtrl pressUp
    je .LCtrlUp

    push eax
    and al, 10000000b
    cmp al, 10000000b
    pop eax
    je ps2_waitKey

    mov bx, asciiTable
    xlatb

    cmp byte [leftShiftPressed], 0
    je .end

    cmp al, 'a'
    jnge .checkNum
    cmp al, 'z'
    jnle .checkNum
    
    sub al, 0x20
    jmp .end

.checkNum:
    cmp al, '0'
    jnge .checkOther
    cmp al, '9'
    jnle .checkOther

    sub al, 0x30
    mov bx, asciiTableNumsShifted
    xlatb
    jmp .end

.checkOther:
    cmp al, '='
    je .checkEqual
    cmp al, '-'
    je .checkMinus
    cmp al, '`'
    je .checkTilda
    cmp al, '['
    je .checkLeftBracket
    cmp al, ']'
    je .checkRightBracket
    cmp al, ';'
    je .checkSemicolon
    cmp al, "'"
    je .checkQuotMarks
    cmp al, ','
    je .checkComma
    cmp al, '.'
    je .checkDot
    cmp al, '/'
    je .checkSlash

    jmp .end

.end:
    ret

.checkEqual:
    mov al, '+'
    jmp .end

.checkMinus:
    mov al, '_'
    jmp .end

.checkTilda:
    mov al, '~'
    jmp .end
.checkLeftBracket:
    mov al, '{'
    jmp .end
.checkRightBracket:
    mov al, '}'
    jmp .end
.checkSemicolon:
    mov al, ':'
    jmp .end
.checkQuotMarks:
    mov al, '"'
    jmp .end
.checkComma:
    mov al, '<'
    jmp .end
.checkDot:
    mov al, '>'
    jmp .end
.checkSlash:
    mov al, '?'
    jmp .end

.LShiftDown:
    mov byte [leftShiftPressed], 1
    jmp ps2_waitKey
.LShiftUp:
    mov byte [leftShiftPressed], 0
    jmp ps2_waitKey
.LCtrlDown:
    mov byte [leftCtrlPressed], 1
    jmp ps2_waitKey
.LCtrlUp:
    mov byte [leftCtrlPressed], 0
    jmp ps2_waitKey

asciiTable: db 0                ; RESERVED
            db 0                ; ESC
            db '1234567890-='
            db 0x08             ; BACKSPACE
            db 0x09             ; TAB
            db 'qwertyuiop[]' 
            db 0xA              ; ENTER
            db 0                ; CTRL
            db "asdfghjkl;'`"
            db 0                ; L SHIFT
            db '\zxcvbnm,./'
            db 0                ; R SHIFT
            db '*'              ; NUMPAD
            db 0                ; ALT
            db ' '
            db 0                ; CAPS LOCK
            db 0, 0, 0, 0, 0, 0 ; F1-F6
            db 0, 0, 0, 0       ; F7-F10
            db 0                ; NUM LOCK
            db 0                ; SCROLL LOCK
            db 0                ; HOME
            db 0
            db 0
            db '-'              ; NUMPAD MINUS
            db 0
            db '5'              ; NUMPAD 5
            db 0
            db '+'              ; KEYPAD PLUS
            db 0                ; END
            db 0
            db 0                ; PgDown
            db 0                ; Insert
            db 0                ; Delete
            db 0                ; PrtSc SysRq
            db '#'              ; Macro
            db 0, 0             ; f11-f12
            db 0, 0, 0          ; f13-f15 (lWin, rWin, Menu)
            db 0, 0, 0, 0, 0, 0, 0, 0, 0    ; F16-f24
            db 0
            db 0
            db 0
            db 0
            db 0
            db 0
asciiTableNumsShifted: db ')!@#$%^&*('

leftShiftPressed: db 0
leftCtrlPressed: db 0