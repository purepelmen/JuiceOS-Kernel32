#include "drivers/screen.hpp"
#include "drivers/ports.hpp"

void ScreenDriver::initialize()
{
    VIDEO_MEMORY = (uint8*) 0xB8000;

    cursorX = 0;
    cursorY = 0;
    printColor = STANDART_SCREEN_COLOR;
}

void ScreenDriver::enable_cursor(uint8 cursor_start, uint8 cursor_end)
{
    port_byte_out(0x03D4, 0x0A);
    port_byte_out(0x03D5, (port_byte_in(0x03D5) & 0xC0) | cursor_start);

    port_byte_out(0x03D4, 0x0B);
    port_byte_out(0x03D5, (port_byte_in(0x03D5) & 0xE0) | cursor_end);
}

void ScreenDriver::disable_cursor()
{
	port_byte_out(0x3D4, 0x0A);
	port_byte_out(0x3D5, 0x20);
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
    update_cursor();
}

void ScreenDriver::print_char(uint8 print_char)
{
    uint8* addressToPrint = (uint8*) VIDEO_MEMORY + (cursorX * 2 + cursorY * 160);

    if(print_char == 0xA)
    {
        // New line
        cursorY += 1;
        cursorX = 0;
    } 
    else if (print_char == 0x08)
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
        addressToPrint[0] = print_char;
        addressToPrint[1] = printColor;

        cursorX += 1;
        if(cursorX == 80)
        {
            cursorY += 1;
            cursorX = 0;
        }
    }

    update_cursor();
    update_scroll();
}

void ScreenDriver::print_char_noupd(uint8 print_char)
{
    uint8* adressToPrint = (uint8*) VIDEO_MEMORY + (cursorX * 2 + cursorY * 160);

    adressToPrint[0] = print_char;
    adressToPrint[1] = printColor;

    cursorX += 1;
    if(cursorX == 80)
    {
        cursorY += 1;
        cursorX = 0;
    }
}

void ScreenDriver::print_string(string str)
{
    for(int i = 0; str[i] != 0x0; i++)
        print_char(str[i]);
}

void ScreenDriver::print_string_noupd(string str)
{
    for(int i = 0; str[i] != 0x0; i++)
        print_char_noupd(str[i]);
}

void ScreenDriver::update_scroll()
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

void ScreenDriver::update_cursor()
{
    uint16 cursorPosition = cursorY * 80 + cursorX;

    port_byte_out(0x03D4, 0x0F);
    port_byte_out(0x03D5, cursorPosition);

    port_byte_out(0x03D4, 0x0E);
    port_byte_out(0x03D5, cursorPosition >> 8);
}

void ScreenDriver::operator<<(string str)
{
    print_string(str);
}

void ScreenDriver::operator<<(uint8 _char)
{
    print_char(_char);
}
