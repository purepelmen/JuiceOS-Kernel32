#include "ports.h"
#include "screen.h"

namespace kscreen
{
    struct out_arguments outargs;

    void out_arguments::set_cursor_pos(int x, int y)
    {
        cursor_x = x;
        cursor_y = y;
    }

    void clear()
    {
        outargs.VIDEO_MEMORY = (uint8*) 0xb8000;
        outargs.print_color = SCREEN_STDCOLOR;
        
        for(int i = 0; i < 80 * 25 * 2; i++)
        {
            outargs.VIDEO_MEMORY[i] = ' ';
            i++;
            
            outargs.VIDEO_MEMORY[i] = outargs.print_color;
        }

        outargs.set_cursor_pos(0, 0);
        update_cursor();
    }

    void update_scroll()
    {
        if(outargs.cursor_y >= 25)
        {
            uint8* si = (uint8*) outargs.VIDEO_MEMORY + (80 * 2);
            uint8* di = (uint8*) outargs.VIDEO_MEMORY;

            for(int i = 0; i < 80 * 24 * 2; i++)
                di[i] = si[i];

            di = (uint8*) outargs.VIDEO_MEMORY + (80 * 24 * 2);
            for(int i = 0; i < 80 * 2; i++)
            {
                di[i] = ' ';
                i++;
                di[i] = outargs.print_color;
            }
            
            outargs.cursor_y = 24;
        }
    }

    void update_cursor()
    {
        uint16 cursorPosition = outargs.cursor_y * 80 + outargs.cursor_x;

        port_write8(0x03D4, 0x0F);
        port_write8(0x03D5, cursorPosition);

        port_write8(0x03D4, 0x0E);
        port_write8(0x03D5, cursorPosition >> 8);
    }

    void enable_cursor(uint8 cursor_start, uint8 cursor_end)
    {
        port_write8(0x03D4, 0x0A);
        port_write8(0x03D5, (port_read8(0x03D5) & 0xC0) | cursor_start);

        port_write8(0x03D4, 0x0B);
        port_write8(0x03D5, (port_read8(0x03D5) & 0xE0) | cursor_end);
    }

    void disable_cursor()
    {
        port_write8(0x3D4, 0x0A);
        port_write8(0x3D5, 0x20);
    }

    void print_char(uint8 print_char)
    {
        uint8* addressToPrint = (uint8*) outargs.VIDEO_MEMORY + (outargs.cursor_x * 2 + outargs.cursor_y * 160);

        if(print_char == 0xA)
        {
            // New line
            outargs.cursor_y += 1;
            outargs.cursor_x = 0;
        } 
        else if (print_char == 0x08)
        {
            // Backspace
            if(outargs.cursor_x != 0)
            {
                outargs.cursor_x -= 1;

                addressToPrint = (uint8*) outargs.VIDEO_MEMORY + (outargs.cursor_x * 2 + outargs.cursor_y * 160);
                addressToPrint[0] = ' ';
                addressToPrint[1] = outargs.print_color;
            }
        } 
        else
        {
            addressToPrint[0] = print_char;
            addressToPrint[1] = outargs.print_color;

            outargs.cursor_x += 1;
            if(outargs.cursor_x == 80)
            {
                outargs.cursor_y += 1;
                outargs.cursor_x = 0;
            }
        }

        update_cursor();
        update_scroll();
    }

    void print_char_noupd(uint8 print_char)
    {
        uint8* adressToPrint = (uint8*) outargs.VIDEO_MEMORY + (outargs.cursor_x * 2 + outargs.cursor_y * 160);

        adressToPrint[0] = print_char;
        adressToPrint[1] = outargs.print_color;

        outargs.cursor_x += 1;
        if(outargs.cursor_x == 80)
        {
            outargs.cursor_y += 1;
            outargs.cursor_x = 0;
        }
    }

    void print_string(string str)
    {
        for(int i = 0; str[i] != 0x0; i++)
            print_char(str[i]);
    }

    void print_string_noupd(string str)
    {
        for(int i = 0; str[i] != 0x0; i++)
            print_char_noupd(str[i]);
    }
}
