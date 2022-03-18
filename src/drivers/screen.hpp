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

    void disableCursor();
    void enableCursor(uint8 cursor_start, uint8 cursor_end);

    void clear();
    void printChar(uint8 printChar);
    void printChar_noupdates(uint8 printChar);
    void printString(string str);
    void printString_noupdates(string str);

    void updateScroll();
    void updateCursor();

    void operator<<(string str);
    void operator<<(uint8 _char);

private:
    uint8* VIDEO_MEMORY; 
};
