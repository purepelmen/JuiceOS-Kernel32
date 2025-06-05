#include "console.h"
#include "heap.h"

#include "drivers/ps2.h"
#include "drivers/screen.h"

namespace kconsole
{
    const size_t INPUT_MAXCHARS = 100;
    static char* input_buffer = nullptr;

    struct cursor_state cursor;

    void clear()
    {
        kscreen::clear();
        
        cursor.posX = cursor.posY = 0;
        cursor.color = SCREEN_STDCOLOR;
        sync_hwcursor();
    }

    void sync_hwcursor()
    {
        kscreen::update_cursor(cursor.posX, cursor.posY);
    }

    void sync_scursor_coords()
    {
        cursor.posX = kscreen::outargs.cursor_x;
        cursor.posY = kscreen::outargs.cursor_y;
    }

    static void scursor_step_back()
    {
        if (kscreen::outargs.cursor_x == 0)
        {
            kscreen::outargs.cursor_y -= 1;
            kscreen::outargs.cursor_x = kscreen::width();
        }

        kscreen::outargs.cursor_x--;
    }

    static void update_scursor()
    {
        kscreen::outargs.set_cursor_pos(cursor.posX, cursor.posY);
        kscreen::outargs.print_color = cursor.color;
    }

    string read_string()
    {
        if (input_buffer == 0)
            input_buffer = kheap::alloc_casted<char>(INPUT_MAXCHARS + 1);

        update_scursor();
        for (int i = 0; true; )
        {
            uint8 key = kps2::read_ascii();
            if (key == 0x0) continue;
            
            if (key == 0xA)
            {
                input_buffer[i] = 0x0;
                break;
            }

            if (key == 0x08)
            {
                if(i <= 0) continue;

                scursor_step_back();
                kscreen::printc(' ');
                scursor_step_back();

                sync_scursor_coords();
                sync_hwcursor();

                i -= 1;
                input_buffer[i] = 0x0;
                continue;
            }
            
            if (i >= INPUT_MAXCHARS) continue;
            
            printc(key);
            input_buffer[i++] = key;
        }

        printc(0xA);
        return string(input_buffer);
    }

    void print(const char* text, size_t maxLength)
    {
        update_scursor();

        int i, start = 0;
        for (i = 0; text[i] != 0x0 && i < maxLength; i++)
        {
            char ch = text[i];
            
            // Is a control ASCII char?
            if (ch < 0x20)
            {
                if (ch == 0x0A)
                {
                    int len = i - start; 
                    if (len > 0)
                        kscreen::print(text + start, len);
                    
                    kscreen::outargs.cursor_y += 1;
                    kscreen::outargs.cursor_x = 0;
                    kscreen::update_scroll();

                    start = i + 1;
                }
            }
        }

        int len = i - start; 
        if (len > 0)
            kscreen::print(text + start, len);

        sync_scursor_coords();
        sync_hwcursor();
    }

    void printc(char ch)
    {
        print(&ch, 1);
    }

    void print_hex(unsigned number, uint8 width)
    {
        if (width > 8)
            width = 8;
        
        char temp[9];
        uint_to_hex(number, temp, width);
        
        print(temp);
    }

    void printf(string str, ...)
    {
        va_list args;

        va_start(args, str);
        vsprintf([](void* context, const char* portionPtr, int length) 
        {
            print(portionPtr, length);
        }, nullptr, str.ptr(), args);

        va_end(args);
    }
}
