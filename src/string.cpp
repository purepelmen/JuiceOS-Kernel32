#include "string.hpp"

string::string(const char* char_sequence)
{
    pointer = (char*) char_sequence;
}

uint32 string::length()
{
    int i;
    for(i = 0; pointer[i] != 0x0; i++);

    return i;
}

bool string::compare_length(string other_str)
{
    return length() == other_str.length();
}

bool string::compare(string other_str)
{
    if(compare_length(other_str) == false) 
        return false;

    bool result = true;
    uint32 i = 0;
    while(pointer[i] != 0x0 || other_str.pointer[i] != 0x0)
    {
        if(pointer[i] == other_str.pointer[i])
        {
            i += 1;
            continue;
        }

        result = false;
        break;
    }

    return result;
}

void string::to_upper(string destination)
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

void string::to_lower(string destination)
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
