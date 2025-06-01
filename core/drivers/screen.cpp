#include "ports.h"
#include "screen.h"

namespace kscreen
{
    uint8* const VIDEO_MEMORY = (uint8*)0xB8000;
    const uint8 WIDTH = 80, HEIGHT = 25;

    struct out_arguments outargs;

    void out_arguments::set_cursor_pos(int x, int y)
    {
        cursor_x = x;
        cursor_y = y;
    }

    void clear()
    {
        outargs.print_color = SCREEN_STDCOLOR;
        
        for (int i = 0; i < WIDTH * HEIGHT * 2; i += 2)
        {
            VIDEO_MEMORY[i] = ' ';
            VIDEO_MEMORY[i + 1] = outargs.print_color;
        }

        outargs.set_cursor_pos(0, 0);
        update_cursor();
    }

    void update_scroll()
    {
        if(outargs.cursor_y >= HEIGHT)
        {
            size_t hzLineInBytes = WIDTH * 2;

            uint8* si = VIDEO_MEMORY + hzLineInBytes;
            uint8* di = VIDEO_MEMORY;
            
            mem_copy(si, di, hzLineInBytes * (HEIGHT - 1));

            di = VIDEO_MEMORY + (hzLineInBytes * (HEIGHT - 1));
            for (int i = 0; i < hzLineInBytes; i += 2)
            {
                di[i] = ' ';
                di[i + 1] = outargs.print_color;
            }
            
            outargs.cursor_y = HEIGHT - 1;
        }
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

    void update_cursor()
    {
        uint16 cursorPosition = outargs.cursor_y * WIDTH + outargs.cursor_x;

        port_write8(0x03D4, 0x0F);
        port_write8(0x03D5, cursorPosition);

        port_write8(0x03D4, 0x0E);
        port_write8(0x03D5, cursorPosition >> 8);
    }

    inline void putc_core(char ch)
    {
        if (outargs.cursor_x >= 80)
        {
            outargs.cursor_y += 1;
            outargs.cursor_x = 0;
        }
        update_scroll();

        uint8* addressToPrint = VIDEO_MEMORY + (outargs.cursor_x * 2 + outargs.cursor_y * WIDTH * 2);
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
