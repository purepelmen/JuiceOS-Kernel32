#pragma once

#include "../stdlib.h"

#define SCREEN_STDCOLOR     0x07
#define SCREEN_INVERTCOLOR  0x70

namespace kscreen
{
    struct out_arguments
    {
        uint8* VIDEO_MEMORY;
        uint8 print_color;

        int cursor_x;
        int cursor_y;

        void set_cursor_pos(int x, int y);
    };

    extern struct out_arguments outargs;

    void clear();

    void update_scroll();
    void update_cursor();

    void enable_cursor(uint8 cursor_start, uint8 cursor_end);
    void disable_cursor();

    void print_char(uint8 print_char, bool updateCursor = true);
    void print_char_raw(uint8 print_char);

    void print_string(string str);
    void print_string_raw(string str);
}
