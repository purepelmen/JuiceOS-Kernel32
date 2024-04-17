#include "console.h"
#include "heap.h"

#include "drivers/ps2.h"
#include "drivers/screen.h"

static char* input_buffer = nullptr;

namespace kconsole
{
    static void print_hex4(uint8 char4bit);
    
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

        return string(input_buffer);
    }

    void print_hex8(uint8 byte)
    {
        print_hex4(byte >> 4);
        print_hex4(byte & 0x0F);
    }

    void print_hex16(uint16 word)
    {
        print_hex8(word >> 8);
        print_hex8(word & 0x00FF);
    }

    void print_hex32(uint32 dword)
    {
        print_hex16(dword >> 16);
        print_hex16(dword & 0x0000FFFF);
    }

    void print_decimal(uint32 num)
    {
        if (num == 0)
        {
            kscreen::print_char('0');
            return;
        }

        int acc = num;
        char c[32];
        int i = 0;
        while(acc > 0)
        {
            c[i] = '0' + acc%10;
            acc /= 10;
            i++;
        }

        c[i] = 0;

        char c2[32];
        c2[i--] = 0;
        int j = 0;
        while(i >= 0)
        {
            c2[i--] = c[j++];
        }

        kscreen::print_string(c2);
    }

    void printf(string str, ...)
    {
        uint32 args_addr = (uint32) &str + sizeof(str);
        uint32* args_ptr = (uint32*) args_addr;

        for(int i = 0; str[i] != 0x0; i++)
        {
            if(str[i] == '%')
            {
                i++;

                if(str[i] == 'd')
                {
                    print_decimal(*(args_ptr));
                    args_ptr += 1;
                }
                else if(str[i] == 'x')
                {
                    print_hex32(*(args_ptr));
                    args_ptr += 1;
                }
                else if(str[i] == 'c')
                {
                    kscreen::print_char(*(args_ptr));
                    args_ptr += 1;
                }
                else if(str[i] == 's')
                {
                    kscreen::print_string((const char*) *(args_ptr));
                    args_ptr += 1;
                }

                continue;
            }

            kscreen::print_char(str[i]);
        }
    }

    void print_hex4(uint8 char4bit)
    {
        char4bit += 0x30;

        if(char4bit <= 0x39)
        {
            kscreen::print_char(char4bit);
        }
        else
        {
            char4bit += 39;
            kscreen::print_char(char4bit);
        }
    }
}
