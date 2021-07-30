#include "stdio.h"
#include "ports.h"
#include "ps2.h"

const uint8_t* asciiTable = "\x00\x1B" "1234567890-=" "\x08\x09" "qwertyuiop[]" "\x0A\x00" "asdfghjkl;'`" "\x00" "\\zxcvbnm,./" "\x00" "*\x00" " " "\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00" "789" "-" "456" "+" "1230" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
const uint8_t* asciiTableNumsShifted = ")!@#$%^&*(";

uint8_t leftShiftPressed = 0;
uint8_t leftCtrlPressed = 0;
uint8_t capsLockActive = 0;

uint8_t ps2_keyboard_getKey(void) {
    while(1) {
        uint8_t ch = port_byte_in(0x64);
        if(ch & 1) break;
    }

    uint8_t result = port_byte_in(0x60);
    return result;
}

uint8_t ps2_waitScancode(uint8_t ignoreReleases) {
    while(1) {
        uint8_t scan = ps2_keyboard_getKey();
        if(ignoreReleases) {
            if(scan & (1 << 7)) 
                continue;
            else
                break;
        } else
            break;
    }
}

uint8_t ps2_waitKey(void) {
    uint8_t ourChar;
    while(1) {
        uint8_t scan = ps2_keyboard_getKey();

        if(scan == 0x2A) {
            leftShiftPressed = 1;
            continue;
        } else if(scan == 0xAA) {
            leftShiftPressed = 0;
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
            ourChar = 0xF0;
            break;
        }

        ourChar = asciiTable[scan];

        if(capsLockActive == 0 && leftShiftPressed == 0)
            break;

        if(ourChar >= 'a' && ourChar <= 'z') {
            ourChar -= 0x20;
            break;
        }
        if(ourChar >= '0' && ourChar <= '9') {
            ourChar -= 0x30;
            ourChar = asciiTableNumsShifted[ourChar];
            break;
        }

        if(ourChar == '=') 
            ourChar = '+';
        if(ourChar == '/') 
            ourChar = '?';
        if(ourChar == '-') 
            ourChar = '_';
        if(ourChar == '`') 
            ourChar = '~';
        if(ourChar == '[') 
            ourChar = '{';
        if(ourChar == ']') 
            ourChar = '}';
        if(ourChar == ';') 
            ourChar = ':';
        if(ourChar == '\'') 
            ourChar = '"';
        if(ourChar == ',') 
            ourChar = '<';
        if(ourChar == '.') 
            ourChar = '>';

        break;
    }
    return ourChar;
}

// UP_ARROW = 48 (c8 - release)
// DOWN_ARROW = 50 (d0 - release)
// LEFT_ARROW = 4b (cb - release)
// RIGHT_ARROW = 4d (cd - release)
// CapsLock = 3a (ba release)
// ESCAPE = 01 (81 release)
// Right Shift = 36 (b6 release)