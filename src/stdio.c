/*
    *
    **
    *** STDIO.C -- provides basic functions for printing strings, getting the input, 
    *** working with strings and memory.
    **
    *
*/

#include "screen.h"
#include "ps2.h"
#include "stdio.h"
#include "heap.h"

uint8 inputExitCode = 0x0;

uint8* get_input() {
    uint8* inputBuffer = malloc(60);
    unsigned int inputBufferCounter = 0;

    // Reset input buffer
    mem_set(inputBuffer, 0x0, 60);

    // Input loop
    while(1) {
        uint8 key = ps2_readKey();

        if(key == 0x0) continue;

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
    return inputBuffer;
}

uint32 str_len(const uint8* string) {
    uint32 i = 0;
    for(; string[i] != 0x0; i++);
    return i;
}

boolean str_copmare_len(const uint8* str1, const uint8* str2) {
    return str_len(str1) == str_len(str2);
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

void print_number(uint32 num) {
    if (num == 0) {
        print_char('0');
        return;
    }

    int acc = num;
    char c[32];
    int i = 0;
    while (acc > 0) {
        c[i] = '0' + acc%10;
        acc /= 10;
        i++;
    }

    c[i] = 0;

    char c2[32];
    c2[i--] = 0;
    int j = 0;
    while(i >= 0) {
        c2[i--] = c[j++];
    }

    print_string(c2);
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