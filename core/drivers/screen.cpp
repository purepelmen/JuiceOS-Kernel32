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

    inline void putc_core(char ch)
    {
        if (outargs.cursor_x >= 80)
        {
            outargs.cursor_y += 1;
            outargs.cursor_x = 0;
        }
        update_scroll();

        uint8* addressToPrint = (uint8*) outargs.VIDEO_MEMORY + (outargs.cursor_x * 2 + outargs.cursor_y * 160);
        addressToPrint[0] = ch;
        addressToPrint[1] = outargs.print_color;

        outargs.cursor_x += 1;
    }

    void putc(unsigned x, unsigned y, char ch)
    {
        outargs.cursor_x = x;
        outargs.cursor_y = y;

        putc_core(ch);
    }

    void printc(char ch)
    {
        putc_core(ch);
    }

    void print(const char* source, size_t bufferLength)
    {
        for (int i = 0; source[i] != 0x0 && i < bufferLength; i++)
        {
            putc_core(source[i]);
        }
    }

    void print(unsigned x, unsigned y, const char* source, size_t bufferLength)
    {
        outargs.cursor_x = x;
        outargs.cursor_y = y;

        print(source, bufferLength);
    }
}
