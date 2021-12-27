/*
    *
    **
    *** STDIO.C -- provides basic functions for getting the input, 
    *** working with strings and memory.
    **
    *
*/

#include "drivers/screen.h"
#include "drivers/ps2.h"

#include "stdlib.h"
#include "heap.h"

uint8* inputBuffer = 0;

uint8* ReadString()
{
    // Allocating 60 bytes for text
    if(inputBuffer == 0)
        inputBuffer = malloc(60);

    // Clearing allocated section
    FillMem(inputBuffer, 0x0, 60);
    
    for(int i = 0; true; )
    {
        uint8 key = Ps2ReadKey();

        if(key == 0x0) continue;
        if(key == 0xA) break;

        if(key == 0x08)
        {
            if(i <= 0) continue;

            PrintChar(key);
            i -= 1;
            inputBuffer[i] = 0x0;
            continue;
        }
        
        if(i >= 58) continue;
        
        PrintChar(key);
        inputBuffer[i] = key;
        i += 1;
    }

    return inputBuffer;
}

uint32 GetStringLength(const uint8* _string)
{
    uint32 i = 0;
    for(; _string[i] != 0x0; i++);

    return i;
}

bool CompareStringLength(const uint8* _string1, const uint8* _string2)
{
    return GetStringLength(_string1) == GetStringLength(_string2);
}

bool CompareString(const uint8* _string1, const uint8* _string2)
{
    if(CompareStringLength(_string1, _string2) == false) 
        return 0;

    uint8 result = 1;
    uint32 i = 0;
    while(_string1[i] != 0x0 || _string2[i] != 0x0)
    {
        if(_string1[i] == _string2[i])
        {
            i += 1;
            continue;
        }

        result = 0;
        break;
    }

    return result;
}

void ToUpperCase(const uint8* original, uint8* destination)
{
    uint32 i = 0;
    while(original[i] != 0x0)
    {
        if(original[i] >= 'a' && original[i] <= 'z')
            destination[i] -= 0x20;
        else
            destination[i] = original[i];

        i++;
    }
}

void ToLowerCase(const uint8* string, uint8* destination)
{
    uint32 i = 0;
    while(string[i] != 0x0)
    {
        if(string[i] >= 'A' && string[i] <= 'Z')
            destination[i] += 0x20;
        else
            destination[i] = string[i];
        
        i++;
    }
}

void ConcatString(uint8* concatTo, uint8* from)
{
    int i = 0;
    while(concatTo[i] != 0x0) i++;

    int i2 = 0;
    while(from[i2] != 0x0)
        concatTo[i++] = from[i2++];
    
    concatTo[i] = 0x0;
}

void PrintHalfByteAsString(uint8 char4bit) {
    char4bit += 0x30;

    if(char4bit <= 0x39)
        PrintChar(char4bit);
    else
    {
        char4bit += 39;
        PrintChar(char4bit);
    }
}

void PrintByteAsString(uint8 byte)
{
    PrintHalfByteAsString(byte >> 4);
    PrintHalfByteAsString(byte & 0x0F);
}

void PrintShortAsString(uint16 word)
{
    PrintByteAsString(word >> 8);
    PrintByteAsString(word & 0x00FF);
}

void PrintIntAsString(uint32 dword)
{
    PrintShortAsString(dword >> 16);
    PrintShortAsString(dword & 0x0000FFFF);
}

void PrintNum(uint32 num)
{
    if (num == 0)
    {
        PrintChar('0');
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

    PrintString(c2);
}

void CopyMem(uint8* source, uint8* destination, uint32 bytesAmount)
{
    for(int i=0; i < bytesAmount; i++)
        destination[i] = source[i];
}

bool CompareMem(uint8* first, uint8* seconds, uint32 bytesAmount)
{
    for(int i=0; i < bytesAmount; i++)
        if(first[i] != seconds[i]) return false;

    return true;
}

void FillMem(uint8* ptr, uint8 byte, uint32 amount)
{
    for(int i=0; i < amount; i++)
        ptr[i] = byte;
}

void RaiseError(const uint8* message, const char *file, uint32 line)
{
    asm volatile("cli");

    PrintString("PANIC (");
    PrintString(message);
    PrintString(") at ");
    PrintString(file);
    PrintString(":");
    PrintIntAsString(line);
    PrintString("\n");
    
    for(;;);
}
