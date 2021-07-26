#ifndef C_PS2_DRIVER_LIB
#define C_PS2_DRIVER_LIB

#include "ports.h"

const unsigned char* asciiTable = "\x00\x00" "1234567890-=" "\x08\x09" "qwertyuiop[]" "\x0A\x00" "asdfghjkl;'`" "\x00" "\\zxcvbnm,./" "\x00" "*\x00" " " "\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00" "-" "\x00" "5" "\x00" "+" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
const unsigned char* asciiTableNumsShifted = ")!@#$%^&*(";

unsigned char leftShiftPressed = 0;
unsigned char leftCtrlPressed = 0;

unsigned char ps2_keyboard_getKey() {
    while(1) {
        char ch = port_byte_in(0x64);
        if(ch & 3) break;
    }

    char result = port_byte_in(0x60);
    return result;
}

unsigned char ps2_waitKey(void) {
    unsigned char ourChar;
    while(1) {
        unsigned char scan = ps2_keyboard_getKey();

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