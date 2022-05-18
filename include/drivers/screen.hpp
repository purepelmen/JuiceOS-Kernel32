#pragma once

#include "../stdlib.hpp"

#define STANDART_SCREEN_COLOR           0x07
#define STANDART_INVERTED_SCREEN_COLOR  0x70
#define STANDART_CONSOLE_PREFIX_COLOR   0x02

class ScreenDriver
{
public:
    uint8 printColor;
    int cursorX;
    int cursorY;

    void initialize();

    void disable_cursor();
    void enable_cursor(uint8 cursor_start, uint8 cursor_end);

    void clear();
    void print_char(uint8 print_char);
    void print_char_noupd(uint8 print_char);
    void print_string(string str);
    void print_string_noupd(string str);

    void update_scroll();
    void update_cursor();

    void operator<<(string str);
    void operator<<(uint8 _char);

private:
    uint8* VIDEO_MEMORY; 
};
