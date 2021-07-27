#ifndef C_STDIO_LIB
#define C_STDIO_LIB

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

void update_cursor(void);
void update_scrolling(void);
void print_char(uint8_t aChar);

#include "types.h"
#include "ports.h"
#include "ps2.h"

uint32_t cursorX = 0;
uint32_t cursorY = 0;
uint8_t printColor = 0x07;

uint8_t* VIDMEM = (uint8_t*) 0xb8000;

void clear_screen(void) {
    unsigned int i = 0;
    while(i < 80 * 25 * 2) {
        VIDMEM[i] = ' ';
        i++;
        VIDMEM[i] = printColor;
        i++;
    }

    cursorX = 0;
    cursorY = 0;
    update_cursor();
}

void print_char(uint8_t aChar) {
    uint8_t* adressToPrint = (uint8_t*) VIDMEM + (cursorX * 2 + cursorY * 160);

    if(aChar == 0xA) {
        // New line
        cursorY += 1;
        cursorX = 0;
    } else if (aChar == 0x08) {
        // Backspace
        if(cursorX != 0) {
            cursorX -= 1;

            adressToPrint = (uint8_t*) VIDMEM + (cursorX * 2 + cursorY * 160);
            adressToPrint[0] = ' ';
            adressToPrint[1] = printColor;
        }
    } else {
        adressToPrint[0] = aChar;
        adressToPrint[1] = printColor;

        cursorX += 1;
        if(cursorX == 80) {
            cursorY += 1;
            cursorX = 0;
        }
    }

    update_scrolling();
    update_cursor();
}

void print_string(const uint8_t* string) {
    uint32_t i = 0;
    while(string[i] != 0x0) {
        print_char(string[i]);
        i += 1;
    }
}

void update_scrolling(void) {
    if(cursorY >= 25) {
        uint8_t* si = (uint8_t*) VIDMEM+(80*2);
        uint8_t* di = (uint8_t*) VIDMEM;

        for(int i=0; i < 80*24*2; i++) {
            di[i] = si[i];
        }

        di = (uint8_t*) VIDMEM+(80*24*2);
        for(int i=0; i < 80*2; i++) {
            di[i] = ' ';
            i++;
            di[i] = printColor;
        }
        cursorY = 24;
    }
}

void update_cursor(void) {
    uint16_t cX = cursorX;
    uint16_t cY = cursorY;

    cY *= 80;
    cX += cY;

    port_byte_out(0x03D4, 0x0F);
    port_byte_out(0x03D5, cX & 0x00FF);

    port_byte_out(0x03D4, 0x0E);
    port_byte_out(0x03D5, cX >> 8);
}

uint8_t* get_input() {
    static uint8_t inputBuffer[60];
    unsigned int inputBufferCounter = 0;

    // Reset input buffer
    uint8_t* di = (uint8_t*) &inputBuffer;
    for(int i=0; i < 60; i++) {
        di[i] = 0x0;
    }

    // Input loop
    while(1) {
        uint8_t key = ps2_waitKey();

        if(key == 0xA) {
            // Enter pressed
            break;
        }

        if(key == 0x08) {
            // Backspace pressed

            if(inputBufferCounter <= 0) {
                continue;
            }

            print_char(key);
            inputBufferCounter -= 1;
            inputBuffer[inputBufferCounter] = 0x0;
            continue;
        } 
        if(inputBufferCounter >= 58) {
            continue;
        }
        print_char(key);
        inputBuffer[inputBufferCounter] = key;
        inputBufferCounter += 1;
    }
    return (uint8_t*) &inputBuffer;
}

uint32_t strlen(const uint8_t* str) {
    uint32_t i = 0;
    while(str[i] != 0x0) {
        i++;
    }
    return i;
}

uint8_t strlen_cmp(const uint8_t* str1, const uint8_t* str2) {
    unsigned fst = strlen(str1);
    unsigned snd = strlen(str2);
    return fst == snd;
}

uint8_t compare_string(const uint8_t* str1, const uint8_t* str2) {
    if(!strlen_cmp(str1, str2)) 
        return 0;

    uint8_t result = 1;
    uint32_t i = 0;
    while(str1[i] != 0x0 || str2[i] != 0x0) {
        if(str1[i] == str2[i]) {
            i += 1;
            continue;
        }
        result = 0;
        break;
    }
    return result;
}

void str_upper(const uint8_t* string, uint8_t* destination) {
    uint32_t i = 0;
    while(string[i] != 0x0) {
        if(string[i] >= 'a' && string[i] <= 'z') {
            destination[i] -= 0x20;
        } else {
            destination[i] = string[i];
        }
        i++;
    }
}

void str_lower(const uint8_t* string, uint8_t* destination) {
    uint32_t i = 0;
    while(string[i] != 0x0) {
        if(string[i] >= 'A' && string[i] <= 'Z') {
            destination[i] += 0x20;
        } else {
            destination[i] = string[i];
        }
        i++;
    }
}

// TODO: Find better solution for using this procedure
void str_split(const uint8_t* string, uint8_t* destination, uint8_t separator, uint32_t index) {
    uint32_t parsingIndex = 0;
    uint32_t destIndex = 0;
    uint32_t i = 0;
    while(string[i] != 0x0) {
        if(index < parsingIndex) {
            break;
        }
        if(string[i] == separator) {
            parsingIndex++;
            i++;
            continue;
        }

        if(index == parsingIndex) {
            destination[destIndex] = string[i];
            destIndex++;
        }
        i++;
    }
}

#endif