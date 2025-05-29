#include "stdlib.h"
#include "stdarg.h"

#include "drivers/screen.h"
#include "console.h"
#include "heap.h"

void mem_copy(const void* source, void* destination, uint32 bytes_amount)
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

void vsprintf(vsprintf_consumer callback, void* context, const char* source, va_list list)
{
    char temp[20];
    const char* start = source;

    int i;
    char ch;
    for (i = 0; (ch = source[i]) != 0x0; i++)
    {
        if (ch != '%')
            continue;
        
        const char* currPtr = &source[i];
        if (currPtr != start)
            callback(context, start, currPtr - start);

        ch = source[++i];

        if(ch == 'd')
        {
            int_to_str(va_arg(list, int), temp);
            callback(context, temp, string(temp).length());
        }
        else if(ch == 'x')
        {
            uint_to_hex(va_arg(list, unsigned), temp, 8);
            callback(context, temp, 8);
        }
        else if(ch == 'c')
        {
            char argChar = va_arg(list, int);
            callback(context, &argChar, 1);
        }
        else if(ch == 's')
        {
            const char* argString = va_arg(list, const char*);
            callback(context, argString, string(argString).length());
        }
        else
        {
            callback(context, "%", 1);
            i--;
        }

        start = &source[i + 1];
    }

    const char* currPtr = &source[i];
    if (currPtr != start)
        callback(context, start, currPtr - start);
}

void sprintf(char* outBuff, int maxOutLength, const char* source, ...)
{
    struct cback_data
    {
        char* outBuff;
        int maxOutLength_noNullTerm;
        
        char* outCurrPtr;

        int totallyCopied() { return outCurrPtr - outBuff; }
    };

    cback_data data;
    data.outBuff = data.outCurrPtr = outBuff;
    data.maxOutLength_noNullTerm = maxOutLength - 1;
    
    auto callback = [](void* context, const char* portionPtr, int length) 
    {
        cback_data* data = (cback_data*)context; 

        int spaceLeft = data->maxOutLength_noNullTerm - data->totallyCopied();

        int overflowAmount = -(spaceLeft - length);
        if (overflowAmount < 0)
            overflowAmount = 0;

        for(int i = 0; i < length - overflowAmount; i++)
        {
            *data->outCurrPtr++ = portionPtr[i];
        }
    };
    
    va_list args;

    va_start(args, source);
    vsprintf(callback, &data, source, args);
    va_end(args);

    if (maxOutLength - data.totallyCopied() >= 1)
        *data.outCurrPtr = 0x0;
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

unsigned str_to_uint(const char *source, int base)
{
    if (base < 2 || base > 16)
        return 0;
    
    unsigned finalNumber = 0;
    char ch;
    while ((ch = *source++) != 0x0)
    {
        // Convert to lower-case if necessary.
        if (ch >= 'a' && ch <= 'f')
            ch -= 'a' - 'A';
        
        unsigned number;
        if (ch >= '0' && ch <= '9')
            number = ch - '0';
        else if (ch >= 'A' && ch <= 'F')
            number = 10 + (ch - 'A');
        else 
            break;

        // Digit number bigger than the base is not a part of the whole number.
        if (number >= base)
            break;

        finalNumber *= base;
        finalNumber += number;
    }

    return finalNumber;
}

int str_to_int(const char *source, int base)
{
    if (base < 2 || base > 16)
        return 0;
    
    int mult = 1;
    if (*source == '-')
    {
        mult = -1;
        source++;
    }
    else if (*source == '+')
        source++;

    return str_to_uint(source, base) * mult;
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
