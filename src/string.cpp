#include "string.hpp"

string::string(const char* charSequence)
{
    pointer = (char*) charSequence;
}

uint32 string::length()
{
    int i;
    for(i = 0; pointer[i] != 0x0; i++);

    return i;
}

bool string::compareLength(string otherStr)
{
    return length() == otherStr.length();
}

bool string::compare(string otherStr)
{
    if(compareLength(otherStr) == false) 
        return false;

    bool result = true;
    uint32 i = 0;
    while(pointer[i] != 0x0 || otherStr.pointer[i] != 0x0)
    {
        if(pointer[i] == otherStr.pointer[i])
        {
            i += 1;
            continue;
        }

        result = false;
        break;
    }

    return result;
}

void string::toUpper(string destination)
{
    int i = 0;
    while(pointer[i] != 0x0)
    {
        if(pointer[i] >= 'a' && pointer[i] <= 'z')
            destination.pointer[i] -= 0x20;
        else
            destination.pointer[i] = pointer[i];

        i++;
    }
}

void string::toLower(string destination)
{
    int i = 0;
    while(pointer[i] != 0x0)
    {
        if(pointer[i] >= 'A' && pointer[i] <= 'Z')
            destination.pointer[i] += 0x20;
        else
            destination.pointer[i] = pointer[i];
        
        i++;
    }
}

void string::concat(string from)
{
    int i = 0;
    while(pointer[i] != 0x0) i++;

    int i2 = 0;
    while(from.pointer[i2] != 0x0)
        pointer[i++] = from.pointer[i2++];
    
    pointer[i] = 0x0;
}

char string::operator[](int element)
{
    return pointer[element];
}

bool string::operator==(string str)
{
    return compare(str);
}
