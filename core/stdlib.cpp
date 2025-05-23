#include "stdlib.h"

#include "drivers/screen.h"
#include "console.h"
#include "heap.h"

void mem_copy(void* source, void* destination, uint32 bytes_amount)
{
    for(int i = 0; i < bytes_amount; i++)
        ((uint8*)destination)[i] = ((uint8*)source)[i];
}

bool mem_compare(uint8* first, uint8* seconds, uint32 bytes_amount)
{
    for(int i = 0; i < bytes_amount; i++)
        if(first[i] != seconds[i]) return false;

    return true;
}

void mem_fill(void* ptr, uint8 byte, uint32 amount)
{
    for(int i = 0; i < amount; i++)
        ((uint8*)ptr)[i] = byte;
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

void strcpy(const char *source, char *dest)
{
    int i = 0;
    while(source[i] != 0x0)
    {
        dest[i] = source[i];
        i++;
    }

    dest[i] = 0x0;
}

void uint_to_hex(unsigned value, char *outBuffer, uint8 width)
{
    const char* HEX_CHARS = "0123456789ABCDEF";

    for (int i = width - 1; i >= 0; i--)
    {
        outBuffer[i] = HEX_CHARS[value & 0xF];
        value >>= 4;
    }
    outBuffer[width] = '\0';
}

void uint_to_str(unsigned value, char *outBuffer, int base)
{
    if (base < 2 || base > 16)
    {
        outBuffer[0] = '?';
        outBuffer[1] = 0x0;
        return;
    }

    if (value == 0)
    {
        *outBuffer++ = '0';
        *outBuffer = 0x0;
        return;
    }

    char temp[32];

    int i = 0;
    while (value != 0)
    {
        int reminder = value % base;
        temp[i++] = reminder < 10 ? ('0' + reminder) : ('A' + reminder - 10);
        value /= base; 
    }

    while (i--)
    {
        *outBuffer++ = temp[i];
    }
    *outBuffer = 0x0;
}

void int_to_str(int value, char *outBuffer, int base)
{
    if (value < 0 && base == 10)
    {
        *outBuffer++ = '-';
        value = -value;
    }

    uint_to_str((unsigned)value, outBuffer, base);
}

void raise_error_begin(string message, const char *file, uint32 line)
{
    asm volatile("cli");

    kscreen::outargs.print_color = 0x47;
    kconsole::printf("PANIC (%s) at %s:%d\n", message, file, line);

    kscreen::outargs.print_color = SCREEN_STDCOLOR;
}

void raise_error_end()
{
    asm volatile("hlt");
    while (true);
}
