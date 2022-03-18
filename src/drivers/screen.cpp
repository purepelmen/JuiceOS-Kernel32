#include "drivers/screen.hpp"
#include "drivers/ports.hpp"

void ScreenDriver::initialize()
{
    VIDEO_MEMORY = (uint8*) 0xB8000;

    cursorX = 0;
    cursorY = 0;
    printColor = STANDART_SCREEN_COLOR;
}

void ScreenDriver::enableCursor(uint8 cursor_start, uint8 cursor_end)
{
    WritePortByte(0x03D4, 0x0A);
    WritePortByte(0x03D5, (ReadPortByte(0x03D5) & 0xC0) | cursor_start);

    WritePortByte(0x03D4, 0x0B);
    WritePortByte(0x03D5, (ReadPortByte(0x03D5) & 0xE0) | cursor_end);
}

void ScreenDriver::disableCursor()
{
	WritePortByte(0x3D4, 0x0A);
	WritePortByte(0x3D5, 0x20);
}

void ScreenDriver::clear()
{
    printColor = STANDART_SCREEN_COLOR;
    
    for(int i = 0; i < 80 * 25 * 2; i++)
    {
        VIDEO_MEMORY[i] = ' ';
        i++;
        
        VIDEO_MEMORY[i] = printColor;
    }

    cursorX = 0;
    cursorY = 0;
    updateCursor();
}

void ScreenDriver::printChar(uint8 _char)
{
    uint8* addressToPrint = (uint8*) VIDEO_MEMORY + (cursorX * 2 + cursorY * 160);

    if(_char == 0xA)
    {
        // New line
        cursorY += 1;
        cursorX = 0;
    } 
    else if (_char == 0x08)
    {
        // Backspace
        if(cursorX != 0)
        {
            cursorX -= 1;

            addressToPrint = (uint8*) VIDEO_MEMORY + (cursorX * 2 + cursorY * 160);
            addressToPrint[0] = ' ';
            addressToPrint[1] = printColor;
        }
    } 
    else
    {
        addressToPrint[0] = _char;
        addressToPrint[1] = printColor;

        cursorX += 1;
        if(cursorX == 80)
        {
            cursorY += 1;
            cursorX = 0;
        }
    }

    updateCursor();
    updateScroll();
}

void ScreenDriver::printChar_noupdates(uint8 printChar)
{
    uint8* adressToPrint = (uint8*) VIDEO_MEMORY + (cursorX * 2 + cursorY * 160);

    adressToPrint[0] = printChar;
    adressToPrint[1] = printColor;

    cursorX += 1;
    if(cursorX == 80)
    {
        cursorY += 1;
        cursorX = 0;
    }
}

void ScreenDriver::printString(string str)
{
    for(int i = 0; str[i] != 0x0; i++)
        printChar(str[i]);
}

void ScreenDriver::printString_noupdates(string str)
{
    for(int i = 0; str[i] != 0x0; i++)
        printChar_noupdates(str[i]);
}

void ScreenDriver::updateScroll()
{
    if(cursorY >= 25)
    {
        uint8* si = (uint8*) VIDEO_MEMORY + (80 * 2);
        uint8* di = (uint8*) VIDEO_MEMORY;

        for(int i = 0; i < 80 * 24 * 2; i++)
            di[i] = si[i];

        di = (uint8*) VIDEO_MEMORY + (80 * 24 * 2);
        for(int i=0; i < 80*2; i++)
        {
            di[i] = ' ';
            i++;
            di[i] = printColor;
        }
        
        cursorY = 24;
    }
}

void ScreenDriver::updateCursor()
{
    uint16 cursorPosition = cursorY * 80 + cursorX;

    WritePortByte(0x03D4, 0x0F);
    WritePortByte(0x03D5, cursorPosition);

    WritePortByte(0x03D4, 0x0E);
    WritePortByte(0x03D5, cursorPosition >> 8);
}

void ScreenDriver::operator<<(string str)
{
    printString(str);
}

void ScreenDriver::operator<<(uint8 _char)
{
    printChar(_char);
}
