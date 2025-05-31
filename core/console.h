#pragma once
#include "string.h"

/*
    Provides an abstraction over keyboard/screen drivers.
*/

namespace kconsole
{
    /* Read input until the Escape or Enter key is pressed, then returns the input as a string. Copy the result after calling,
        the output string is reused every time this function is called! */
    string read_string();

    void print(const char* text);
    void printc(char ch);

    void print_hex(unsigned number, uint8 width);

    /* Print a formated string. */
    void printf(string str, ...);
}
