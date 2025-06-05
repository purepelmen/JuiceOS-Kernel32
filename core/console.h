#pragma once
#include "string.h"

/*
    Provides an abstraction over keyboard/screen drivers.
*/

namespace kconsole
{
    struct cursor_state
    {
        unsigned posX, posY;
        uint8 color;
    };

    extern struct cursor_state cursor;

    void clear();

    void sync_hwcursor();
    void sync_scursor_coords();

    /* Read input until the Escape or Enter key is pressed, then returns the input as a string. Copy the result after calling,
        the output string is reused every time this function is called! */
    string read_string();

    void print(const char* text, size_t maxLength = 0xFFFF);
    void printc(char ch);

    void print_hex(unsigned number, uint8 width);

    /* Print a formated string. */
    void printf(string str, ...);
}
