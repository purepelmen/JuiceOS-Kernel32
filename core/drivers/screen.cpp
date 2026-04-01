#include "ports.h"
#include "screen.h"

namespace kscreen
{
    uint16* const VIDEO_MEMORY = (uint16*)0xB8000;
    const uint8 WIDTH = 80, HEIGHT = 25;

    struct out_arguments outargs;

    void out_arguments::set_cursorXY(unsigned x, unsigned y)
    {
        cursor_x = x;
        cursor_y = y;
    }

    size_t width()
    {
        return WIDTH;
    }

    size_t height()
    {
        return HEIGHT;
    }

    void clear()
    {
        outargs.print_color = KSCREEN_STDCOLOR;
        for (int i = 0; i < WIDTH * HEIGHT; i += 1)
        {
            VIDEO_MEMORY[i] = (outargs.print_color << 8) | (uint8)' ';
        }

        outargs.set_cursorXY(0, 0);
    }

    void update_scroll(unsigned* y)
    {
        if(*y >= HEIGHT)
        {
            uint16* si = VIDEO_MEMORY + WIDTH;
            uint16* di = VIDEO_MEMORY;
            
            size_t hzLineInBytes = WIDTH * 2;
            mem_copy(si, di, hzLineInBytes * (HEIGHT - 1));

            di = VIDEO_MEMORY + WIDTH * (HEIGHT - 1);
            for (int i = 0; i < WIDTH; i += 1)
            {
                di[i] = (outargs.print_color << 8) | (uint8)' ';
            }
            
            *y = HEIGHT - 1;
        }
    }

    void enable_hwcursor(uint8 cursor_start, uint8 cursor_end)
    {
        port_write8(0x03D4, 0x0A);
        port_write8(0x03D5, (port_read8(0x03D5) & 0xC0) | cursor_start);

        port_write8(0x03D4, 0x0B);
        port_write8(0x03D5, (port_read8(0x03D5) & 0xE0) | cursor_end);
    }

    void disable_hwcursor()
    {
        port_write8(0x3D4, 0x0A);
        port_write8(0x3D5, 0x20);
    }

    void update_hwcursor(uint16 x, uint16 y)
    {
        uint16 cursorPosition = y * WIDTH + x;

        port_write8(0x03D4, 0x0F);
        port_write8(0x03D5, cursorPosition);

        port_write8(0x03D4, 0x0E);
        port_write8(0x03D5, cursorPosition >> 8);
    }

    inline void putc_core(unsigned* posX, unsigned* posY, char ch)
    {
        if (*posX >= WIDTH)
        {
            *posY += 1;
            *posX = 0;
        }
        update_scroll(posY);

        uint16* addressToPrint = VIDEO_MEMORY + ((*posX) + (*posY) * WIDTH);
        *addressToPrint = (outargs.print_color << 8) | ch & 0xFF;

        *posX += 1;
    }

    void putc(unsigned x, unsigned y, char ch)
    {
        putc_core(&x, &y, ch);
    }

    void printc(char ch)
    {
        putc_core(&outargs.cursor_x, &outargs.cursor_y, ch);
    }

    void print(const char* source, size_t bufferLength)
    {
        for (int i = 0; source[i] != 0x0 && i < bufferLength; i++)
        {
            putc_core(&outargs.cursor_x, &outargs.cursor_y, source[i]);
        }
    }

    void print(unsigned x, unsigned y, const char* source, size_t bufferLength)
    {
        for (int i = 0; source[i] != 0x0 && i < bufferLength; i++)
        {
            putc_core(&x, &y, source[i]);
        }
    }
}
