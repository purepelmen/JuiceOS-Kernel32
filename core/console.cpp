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
        uint32 args_addr = (uint32) &str + sizeof(str);
        uint32* args_ptr = (uint32*) args_addr;

        char temp[20];
        for(int i = 0; str[i] != 0x0; i++)
        {
            if (str[i] != '%')
            {
                kscreen::print_char(str[i], false);
                continue;
            }

            i++;

            if(str[i] == 'd')
            {
                int_to_str(*(args_ptr), temp, 10);
                kscreen::print_string(temp);

                args_ptr += 1;
            }
            else if(str[i] == 'x')
            {
                uint_to_hex(*(args_ptr), temp, 8);
                kscreen::print_string(temp);

                args_ptr += 1;
            }
            else if(str[i] == 'c')
            {
                kscreen::print_char(*(args_ptr), false);
                args_ptr += 1;
            }
            else if(str[i] == 's')
            {
                kscreen::print_string((const char*) *(args_ptr));
                args_ptr += 1;
            }
        }

        kscreen::update_cursor();
    }
}
