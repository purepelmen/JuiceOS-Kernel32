ps2_keyboard_getkey:
    in al, 0x64                 ; 64h port returns 0 if any key is pressed now
    cmp al, 1                   ; Check if it is not pressed
    je ps2_keyboard_getkey      ; Loop 

    in al, 0x60                 ; 60h port - returns the latest pressed key
    ret