#include "inc/stdio.h"
#include "inc/ports.h"
#include "inc/ps2.h"

const uint8_t* asciiTable = "\x00\x1B" "1234567890-=" "\x08\x09" "qwertyuiop[]" "\x0A\x00" "asdfghjkl;'`" "\x00" "\\zxcvbnm,./" "\x00" "*\x00" " " "\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00" "789" "-" "456" "+" "1230" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
const uint8_t* asciiTableNumsShifted = ")!@#$%^&*(";

uint8_t shiftPressed = 0;
uint8_t leftCtrlPressed = 0;
uint8_t capsLockActive = 0;

uint8_t ps2_scancode(uint8_t ignoreReleases) {
    while(1) {
        uint8_t scan = port_byte_in(0x64);
        if(!(scan & 1)) {
            asm volatile("hlt");
            continue;
        }

        if(ignoreReleases) {
            if(scan & (1 << 7)) 
                continue;
            else
                break;
        } else
            break;
    }

    uint8_t result = port_byte_in(0x60);
    return result;
}

uint8_t ps2_readKey(void) {
    uint8_t _char;
    while(1) {
        uint8_t scan = ps2_scancode(false);

        if(scan == 0x2A || scan == 0x36) {
            shiftPressed = 1;
            continue;
        } else if(scan == 0xAA || scan == 0xB6) {
            shiftPressed = 0;
            continue;
        } else if (scan == 0x1D) {
            leftCtrlPressed = 1;
            continue;
        } else if(scan == 0x9D) {
            leftCtrlPressed = 0;
            continue;
        } else if(scan & (1 << 7)) {
            // Key release
            continue;
        } else if(scan == 0x3A) {
            capsLockActive = !capsLockActive;
            continue;
        } else if(scan == 0xFA) {
            // On command sent
            continue;
        }
        
        if(scan == 0x5b) {
            _char = 0xF0;
            break;
        }

        _char = asciiTable[scan];

        if(capsLockActive == 0 && shiftPressed == 0)
            break;

        if(_char >= 'a' && _char <= 'z') {
            _char -= 0x20;
            break;
        }
        if(_char >= '0' && _char <= '9') {
            _char -= 0x30;
            _char = asciiTableNumsShifted[_char];
            break;
        }

        if(_char == '=') 
            _char = '+';
        if(_char == '/') 
            _char = '?';
        if(_char == '-') 
            _char = '_';
        if(_char == '`') 
            _char = '~';
        if(_char == '[') 
            _char = '{';
        if(_char == ']') 
            _char = '}';
        if(_char == ';') 
            _char = ':';
        if(_char == '\'') 
            _char = '"';
        if(_char == ',') 
            _char = '<';
        if(_char == '.') 
            _char = '>';

        break;
    }
    return _char;
}

// UP_ARROW = 48 (c8 - release)
// DOWN_ARROW = 50 (d0 - release)
// LEFT_ARROW = 4b (cb - release)
// RIGHT_ARROW = 4d (cd - release)
// CapsLock = 3a (ba release)
// ESCAPE = 01 (81 release)
// Right Shift = 36 (b6 release)