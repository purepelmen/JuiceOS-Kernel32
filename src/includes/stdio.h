#ifndef C_STDIO_LIB
#define C_STDIO_LIB

#include "ports.h"
#include "ps2.h"

unsigned int cursorX = 0;
unsigned int cursorY = 0;
unsigned char printColor = 0x07;

unsigned char* VIDMEM = (char*) 0xb8000;

void update_cursor(void);
void update_scrolling(void);

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

void print_char(unsigned char aChar) {
    unsigned char* adressToPrint = (char*) VIDMEM + (cursorX * 2 + cursorY * 160);

    if(aChar == 0xA) {
        // New line
        cursorY += 1;
        cursorX = 0;
    } else if (aChar == 0x08) {
        // Backspace
        if(cursorX != 0) {
            cursorX -= 1;

            adressToPrint = (char*) VIDMEM + (cursorX * 2 + cursorY * 160);
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

void print_string(const char* string) {
    unsigned int i = 0;
    while(string[i] != 0x0) {
        print_char(string[i]);
        i += 1;
    }
}

void update_scrolling(void) {
    if(cursorY >= 25) {
        unsigned char* si = (char*) VIDMEM+(80*2);
        unsigned char* di = (char*) VIDMEM;

        for(int i=0; i < 80*24*2; i++) {
            di[i] = si[i];
        }

        di = (char*) VIDMEM+(80*24*2);
        for(int i=0; i < 80*2; i++) {
            di[i] = ' ';
            i++;
            di[i] = printColor;
        }
        cursorY = 24;
    }
}

void update_cursor(void) {
    unsigned short cX = cursorX;
    unsigned short cY = cursorY;

    cY *= 80;
    cX += cY;

    port_byte_out(0x03D4, 0x0F);
    port_byte_out(0x03D5, cX & 0x00FF);

    port_byte_out(0x03D4, 0x0E);
    port_byte_out(0x03D5, cX >> 8);
}

unsigned char* get_input() {
    static unsigned char inputBuffer[60];
    unsigned int inputBufferCounter = 0;

    // Reset input buffer
    unsigned char* di = (char*) &inputBuffer;
    for(int i=0; i < 60; i++) {
        di[i] = 0x0;
    }

    // Input loop
    while(1) {
        unsigned char key = ps2_waitKey();

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
    return (char*) &inputBuffer;
}

unsigned int strlen(const char* str) {
    unsigned int i = 0;
    while(str[i] != 0x0) {
        i++;
    }
    return i;
}

unsigned char strlen_cmp(const char* str1, const char* str2) {
    unsigned fst = strlen(str1);
    unsigned snd = strlen(str2);
    return fst == snd;
}

unsigned char compare_string(const char* str1, const char* str2) {
    if(!strlen_cmp(str1, str2)) 
        return 0;

    unsigned char result = 1;
    unsigned int i = 0;
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

void str_upper(const char* string, char* destination) {
    unsigned int i = 0;
    while(string[i] != 0x0) {
        if(string[i] >= 'a' && string[i] <= 'z') {
            destination[i] -= 0x20;
        } else {
            destination[i] = string[i];
        }
        i++;
    }
}

void str_lower(const char* string, char* destination) {
    unsigned int i = 0;
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
void str_split(const unsigned char* string, unsigned char* destination, unsigned char separator, unsigned int index) {
    unsigned int parsingIndex = 0;
    unsigned int destIndex = 0;
    unsigned int i = 0;
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

#include "types.h"
#endif