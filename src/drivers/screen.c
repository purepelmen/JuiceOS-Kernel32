#include "../stdio.h"
#include "screen.h"

uint32 cursorX = 0;
uint32 cursorY = 0;
uint8 printColor = STANDART_SCREEN_COLOR;

uint8* VIDMEM = (uint8*) 0xB8000;

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

void update_cursor(void) {
    uint16 cursorPosition = cursorY * 80 + cursorX;

    port_byte_out(0x03D4, 0x0F);
    port_byte_out(0x03D5, cursorPosition);

    port_byte_out(0x03D4, 0x0E);
    port_byte_out(0x03D5, cursorPosition >> 8);
}

void clear_screen(void) {
    printColor = STANDART_SCREEN_COLOR;
    
    for(int i=0; i < 80 * 25 * 2; i++) {
        VIDMEM[i] = ' ';
        i++;
        VIDMEM[i] = printColor;
    }

    cursorX = 0;
    cursorY = 0;
    update_cursor();
}

void print_char(uint8 _char) {
    uint8* adressToPrint = (uint8*) VIDMEM + (cursorX * 2 + cursorY * 160);

    if(_char == 0xA) {
        // New line
        cursorY += 1;
        cursorX = 0;
    } else if (_char == 0x08) {
        // Backspace
        if(cursorX != 0) {
            cursorX -= 1;

            adressToPrint = (uint8*) VIDMEM + (cursorX * 2 + cursorY * 160);
            adressToPrint[0] = ' ';
            adressToPrint[1] = printColor;
        }
    } else {
        adressToPrint[0] = _char;
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
    for(int i=0; string[i] != 0x0; i++) {
        print_char(string[i]);
    }
}

void print_string_noupdates(const uint8* string) {
    for(int i=0; string[i] != 0x0; i++) {
        print_char_noupdates(string[i]);
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