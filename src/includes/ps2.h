#ifndef C_PS2_DRIVER_LIB
#define C_PS2_DRIVER_LIB

#include "types.h"
#include "ports.h"

const uint8_t* asciiTable = "\x00\x00" "1234567890-=" "\x08\x09" "qwertyuiop[]" "\x0A\x00" "asdfghjkl;'`" "\x00" "\\zxcvbnm,./" "\x00" "*\x00" " " "\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00" "-" "\x00" "5" "\x00" "+" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
const uint8_t* asciiTableNumsShifted = ")!@#$%^&*(";

uint8_t leftShiftPressed = 0;
uint8_t leftCtrlPressed = 0;

uint8_t ps2_keyboard_getKey() {
    while(1) {
        uint8_t ch = port_byte_in(0x64);
        if(ch & 3) break;
    }

    uint8_t result = port_byte_in(0x60);
    return result;
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
        }

        ourChar = asciiTable[scan];

        if(leftShiftPressed == 0) break;

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

#endif