#include "stdlib.h"

#include "drivers/screen.h"
#include "console.h"
#include "heap.h"

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

    kscreen::outargs.print_color = 0x47;
    kconsole::printf("PANIC (%s) at %s:%d\n", message, file, line);

    asm volatile("hlt");
    while (true);
}
