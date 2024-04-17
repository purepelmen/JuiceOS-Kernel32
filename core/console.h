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

    /* Print a 8-bit number as HEX. */
    void print_hex8(uint8 byte);

    /* Print a 16-bit number as HEX */
    void print_hex16(uint16 word);

    /* Print a 32-bit number as HEX */
    void print_hex32(uint32 dword);

    /* Print a 32-bit number as a decimal number. */
    void print_decimal(uint32 num);

    /* Print a formated string. */
    void printf(string str, ...);
}
