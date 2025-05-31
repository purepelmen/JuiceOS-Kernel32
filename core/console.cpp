#include "console.h"
#include "heap.h"

#include "drivers/ps2.h"
#include "drivers/screen.h"

static char* input_buffer = nullptr;

namespace kconsole
{
    string read_string()
    {
        // Allocating 60 bytes for text
        if(input_buffer == 0)
            input_buffer = kheap::alloc_casted<char>(60);

        for(int i = 0; true; )
        {
            uint8 key = kps2::read_ascii();
            if(key == 0x0) continue;
            
            if(key == 0xA)
            {
                input_buffer[i] = 0x0;
                break;
            }

            if(key == 0x08)
            {
                if(i <= 0) continue;

                kscreen::print_char(key);
                i -= 1;
                input_buffer[i] = 0x0;
                continue;
            }
            
            if(i >= 58) continue;
            
            kscreen::print_char(key);
            input_buffer[i] = key;
            i += 1;
        }

        kscreen::print_char(0xA);
        return string(input_buffer);
    }

    void print(const char* text)
    {
        int i, start = 0;
        for (i = 0; text[i] != 0x0; i++)
        {
            char ch = text[i];
            
            // Is a control ASCII char?
            if (ch < 0x20)
            {
                int len = i - start; 
                if (len > 0)
                    kscreen::print(text + start, len);

                if (ch == 0x0A)
                {
                    kscreen::outargs.cursor_y += 1;
                    kscreen::outargs.cursor_x = 0;
                    // kscreen::update_scroll();
                }
                
                start = i + 1;
            }
        }

        int len = i - start; 
        if (len > 0)
            kscreen::print(text + start, len);

        kscreen::update_cursor();
    }

    void printc(char ch)
    {
        char temp[] = { ch, 0x0 };
        print(temp);
    }

    void print_hex(unsigned number, uint8 width)
    {
        if (width > 8)
            width = 8;
        
        char temp[9];
        uint_to_hex(number, temp, width);
        
        kscreen::print_string(temp);
    }

    void printf(string str, ...)
    {
        va_list args;

        va_start(args, str);
        vsprintf([](void* context, const char* portionPtr, int length) 
        {
            for(int i = 0; i < length; i++)
                kscreen::print_char(portionPtr[i], false);
        }, nullptr, str.ptr(), args);

        va_end(args);
        kscreen::update_cursor();
    }
}
