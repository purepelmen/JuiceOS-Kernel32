#pragma once

#include "../stdlib.h"

#define KSCREEN_STDCOLOR     0x07
#define KSCREEN_INVERTCOLOR  0x70

namespace kscreen
{
    struct out_arguments
    {
        uint8 print_color;

        unsigned cursor_x;
        unsigned cursor_y;

        void set_cursorXY(unsigned x, unsigned y);
    };

    extern struct out_arguments outargs;

    size_t width();
    size_t height();

    void clear();
    void update_scroll(unsigned* y);
    inline void update_scroll() { update_scroll(&outargs.cursor_y); }
    
    void enable_hwcursor(uint8 cursor_start, uint8 cursor_end);
    void disable_hwcursor();
    void update_hwcursor(uint16 x, uint16 y);

    void putc(unsigned x, unsigned y, char ch);
    void printc(char ch);
    
    void print(const char* source, size_t bufferLength = 0xFFFF);
    void print(unsigned x, unsigned y, const char* source, size_t bufferLength = 0xFFFF);
}
