#include "inc/ports.h"
#include "inc/ps2.h"
#include "inc/stdio.h"

uint32_t cursorX = 0;
uint32_t cursorY = 0;
uint8_t printColor = STANDART_SCREEN_COLOR;
uint8_t inputExitCode = 0x0;

uint8_t* VIDMEM = (uint8_t*) 0xb8000;

void clear_screen(void) {
    printColor = STANDART_SCREEN_COLOR;
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

void print_char_noupdates(uint8_t aChar) {
    uint8_t* adressToPrint = (uint8_t*) VIDMEM + (cursorX * 2 + cursorY * 160);

    adressToPrint[0] = aChar;
    adressToPrint[1] = printColor;

    cursorX += 1;
    if(cursorX == 80) {
        cursorY += 1;
        cursorX = 0;
    }
}

void print_string(const uint8_t* string) {
    uint32_t i = 0;
    while(string[i] != 0x0) {
        print_char(string[i]);
        i += 1;
    }
}

void print_string_noupdates(const uint8_t* string) {
    uint32_t i = 0;
    while(string[i] != 0x0) {
        print_char_noupdates(string[i]);
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
    uint16_t cursorPosition = cursorY * 80 + cursorX;

    port_byte_out(0x03D4, 0x0F);
    port_byte_out(0x03D5, cursorPosition);

    port_byte_out(0x03D4, 0x0E);
    port_byte_out(0x03D5, cursorPosition >> 8);
}

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    port_byte_out(0x03D4, 0x0A);
    port_byte_out(0x03D5, (port_byte_in(0x03D5) & 0xC0) | cursor_start);

    port_byte_out(0x03D4, 0x0B);
    port_byte_out(0x03D5, (port_byte_in(0x03D5) & 0xE0) | cursor_end);
}

void disable_cursor(void) {
	port_byte_out(0x3D4, 0x0A);
	port_byte_out(0x3D5, 0x20);
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

        if(key == 0x0) continue;

        if(key == 0xA) {
            // Enter pressed
            inputExitCode = key;
            break;
        }

        if(key == 0xF0) {
            // Windows Left pressed
            inputExitCode = key;
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

void str_concat(uint8_t* concatTo, uint8_t* from) {
    int i = 0;
    while(concatTo[i] != 0x0) i++;

    int i2 = 0;
    while(from[i2] != 0x0)
        concatTo[i++] = from[i2++];
    
    concatTo[i] = 0x0;
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

void print_hex4bit(uint8_t char4bit) {
    char4bit += 0x30;
    if(char4bit <= 0x39) {
        print_char(char4bit);
    } else {
        char4bit += 39;
        print_char(char4bit);
    }
}

void print_hexb(uint8_t byte) {
    print_hex4bit(byte >> 4);
    print_hex4bit(byte & 0x0F);
}

void print_hexw(uint16_t word) {
    print_hexb(word >> 8);
    print_hexb(word & 0x00FF);
}

void print_hexdw(uint32_t dword) {
    print_hexw(dword >> 16);
    print_hexw(dword & 0x0000FFFF);
}

void memcopy(uint8_t* source, uint8_t* destination, uint32_t bytesAmount) {
    for(int i=0; i < bytesAmount; i++) {
        destination[i] = source[i];
    }
}

void mem_set(uint8_t* ptr, uint8_t byte, uint32_t amount) {
    for(int i=0; i < amount; i++) {
        ptr[i] = byte;
    }
}