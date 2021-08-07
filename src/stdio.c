/*
    *
    **
    *** STDIO.C -- provides basic functions for printing strings, getting the input, 
    *** working with strings and memory.
    **
    *
*/

#include "inc/ports.h"
#include "inc/ps2.h"
#include "inc/stdio.h"

uint32 cursorX = 0;
uint32 cursorY = 0;
uint8 printColor = STANDART_SCREEN_COLOR;
uint8 inputExitCode = 0x0;

uint8* VIDMEM = (uint8*) 0xb8000;

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

void print_char(uint8 aChar) {
    uint8* adressToPrint = (uint8*) VIDMEM + (cursorX * 2 + cursorY * 160);

    if(aChar == 0xA) {
        // New line
        cursorY += 1;
        cursorX = 0;
    } else if (aChar == 0x08) {
        // Backspace
        if(cursorX != 0) {
            cursorX -= 1;

            adressToPrint = (uint8*) VIDMEM + (cursorX * 2 + cursorY * 160);
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

void print_char_noupdates(uint8 aChar) {
    uint8* adressToPrint = (uint8*) VIDMEM + (cursorX * 2 + cursorY * 160);

    adressToPrint[0] = aChar;
    adressToPrint[1] = printColor;

    cursorX += 1;
    if(cursorX == 80) {
        cursorY += 1;
        cursorX = 0;
    }
}

void print_string(const uint8* string) {
    uint32 i = 0;
    while(string[i] != 0x0) {
        print_char(string[i]);
        i += 1;
    }
}

void print_string_noupdates(const uint8* string) {
    uint32 i = 0;
    while(string[i] != 0x0) {
        print_char_noupdates(string[i]);
        i += 1;
    }
}

void update_scrolling(void) {
    if(cursorY >= 25) {
        uint8* si = (uint8*) VIDMEM+(80*2);
        uint8* di = (uint8*) VIDMEM;

        for(int i=0; i < 80*24*2; i++) {
            di[i] = si[i];
        }

        di = (uint8*) VIDMEM+(80*24*2);
        for(int i=0; i < 80*2; i++) {
            di[i] = ' ';
            i++;
            di[i] = printColor;
        }
        cursorY = 24;
    }
}

void update_cursor(void) {
    uint16 cursorPosition = cursorY * 80 + cursorX;

    port_byte_out(0x03D4, 0x0F);
    port_byte_out(0x03D5, cursorPosition);

    port_byte_out(0x03D4, 0x0E);
    port_byte_out(0x03D5, cursorPosition >> 8);
}

void enable_cursor(uint8 cursor_start, uint8 cursor_end) {
    port_byte_out(0x03D4, 0x0A);
    port_byte_out(0x03D5, (port_byte_in(0x03D5) & 0xC0) | cursor_start);

    port_byte_out(0x03D4, 0x0B);
    port_byte_out(0x03D5, (port_byte_in(0x03D5) & 0xE0) | cursor_end);
}

void disable_cursor(void) {
	port_byte_out(0x3D4, 0x0A);
	port_byte_out(0x3D5, 0x20);
}

uint8* get_input() {
    static uint8 inputBuffer[60];
    unsigned int inputBufferCounter = 0;

    // Reset input buffer
    mem_set((uint8*) &inputBuffer, 0x0, 60);

    // Input loop
    while(1) {
        uint8 key = ps2_readKey();

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
    return (uint8*) &inputBuffer;
}

uint32 str_len(const uint8* str) {
    uint32 i = 0;
    while(str[i] != 0x0) {
        i++;
    }
    return i;
}

boolean str_copmare_len(const uint8* str1, const uint8* str2) {
    unsigned fst = str_len(str1);
    unsigned snd = str_len(str2);
    return fst == snd;
}

boolean str_compare(const uint8* str1, const uint8* str2) {
    if(!str_copmare_len(str1, str2)) 
        return 0;

    uint8 result = 1;
    uint32 i = 0;
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

void str_upper(const uint8* string, uint8* destination) {
    uint32 i = 0;
    while(string[i] != 0x0) {
        if(string[i] >= 'a' && string[i] <= 'z') {
            destination[i] -= 0x20;
        } else {
            destination[i] = string[i];
        }
        i++;
    }
}

void str_lower(const uint8* string, uint8* destination) {
    uint32 i = 0;
    while(string[i] != 0x0) {
        if(string[i] >= 'A' && string[i] <= 'Z') {
            destination[i] += 0x20;
        } else {
            destination[i] = string[i];
        }
        i++;
    }
}

void str_concat(uint8* concatTo, uint8* from) {
    int i = 0;
    while(concatTo[i] != 0x0) i++;

    int i2 = 0;
    while(from[i2] != 0x0)
        concatTo[i++] = from[i2++];
    
    concatTo[i] = 0x0;
}

// TODO: Find better solution for using this procedure
void str_split(const uint8* string, uint8* destination, uint8 separator, uint32 index) {
    uint32 parsingIndex = 0;
    uint32 destIndex = 0;
    uint32 i = 0;
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

void print_hex4bit(uint8 char4bit) {
    char4bit += 0x30;
    if(char4bit <= 0x39) {
        print_char(char4bit);
    } else {
        char4bit += 39;
        print_char(char4bit);
    }
}

void print_hexb(uint8 byte) {
    print_hex4bit(byte >> 4);
    print_hex4bit(byte & 0x0F);
}

void print_hexw(uint16 word) {
    print_hexb(word >> 8);
    print_hexb(word & 0x00FF);
}

void print_hexdw(uint32 dword) {
    print_hexw(dword >> 16);
    print_hexw(dword & 0x0000FFFF);
}

void mem_copy(uint8* source, uint8* destination, uint32 bytesAmount) {
    for(int i=0; i < bytesAmount; i++) {
        destination[i] = source[i];
    }
}

void mem_set(uint8* ptr, uint8 byte, uint32 amount) {
    for(int i=0; i < amount; i++) {
        ptr[i] = byte;
    }
}

void panic(const uint8* message, const char *file, uint32 line) {
    asm volatile("cli");
    print_string("PANIC (");
    print_string(message);
    print_string(") at ");
    print_string(file);
    print_string(":");
    print_hexdw(line);
    print_string("\n");
    for(;;);
}

void showError(const uint8* error, const uint8* causedBy, const uint8* handledBy, const uint8* details) {
    printColor = 0x04;
    print_string("SYSTEM ERROR! (");
    printColor = 0x0F;
    print_string(error);
    printColor = 0x04;
    print_string(")");
    print_string("\n");

    print_string("Caused by: ");
    printColor = 0x0F;
    print_string(causedBy);
    print_string("\n");

    printColor = 0x04;
    print_string("Handled by: ");
    printColor = 0x0F;
    print_string(handledBy);
    print_string("\n");

    if(str_len(details) > 0) {
        printColor = 0x04;
        print_string("Detailed information: ");
        printColor = 0x0F;
        print_string(details);
        print_string("\n");
    }

    printColor = STANDART_SCREEN_COLOR;
}