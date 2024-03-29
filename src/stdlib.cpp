#include "drivers/screen.hpp"
#include "drivers/ps2.hpp"

#include "stdlib.hpp"
#include "heap.hpp"

uint8* input_buffer = 0;

string read_string()
{
    // Allocating 60 bytes for text
    if(input_buffer == 0)
        input_buffer = (uint8*) malloc(60);

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

    return string((const char*) input_buffer);
}

void print_hex_4bit(uint8 char4bit)
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

void print_hex_8bit(uint8 byte)
{
    print_hex_4bit(byte >> 4);
    print_hex_4bit(byte & 0x0F);
}

void print_hex_16bit(uint16 word)
{
    print_hex_8bit(word >> 8);
    print_hex_8bit(word & 0x00FF);
}

void print_hex_32bit(uint32 dword)
{
    print_hex_16bit(dword >> 16);
    print_hex_16bit(dword & 0x0000FFFF);
}

void print_number(uint32 num)
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
                print_number(*(args_ptr));
                args_ptr += 1;
            }
            else if(str[i] == 'x')
            {
                print_hex_32bit(*(args_ptr));
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

void mem_copy(uint8* source, uint8* destination, uint32 bytes_amount)
{
    for(int i = 0; i < bytes_amount; i++)
        destination[i] = source[i];
}

bool mem_compare(uint8* first, uint8* seconds, uint32 bytes_amount)
{
    for(int i = 0; i < bytes_amount; i++)
        if(first[i] != seconds[i]) return false;

    return true;
}

void mem_fill(uint8* ptr, uint8 byte, uint32 amount)
{
    for(int i = 0; i < amount; i++)
        ptr[i] = byte;
}

void utf16_to_ascii(uint8* buffer, uint16* utf16_str)
{
    int i = 0;
    while(utf16_str[i] != 0x0)
    {
        buffer[i] = utf16_str[i] & 0xFF;
        i++;
    }

    buffer[i] = 0x0;
}

void raise_error(string message, const char *file, uint32 line)
{
    asm volatile("cli");

    kscreen::print_string("PANIC (");
    kscreen::print_string(message);
    kscreen::print_string(") at ");
    kscreen::print_string(file);
    kscreen::print_string(":");
    print_number(line);
    
    kscreen::print_char('\n');
    
    for(;;);
}
