#pragma once

#include "stdint.h"

class string
{
public:
    string(const char* charSequence);
    uint32 length();

    bool compare(string otherStr);
    bool compareLength(string otherStr);

    void toUpper(string destination);
    void toLower(string destination);

    void concat(string from);

    char operator[](int element);

private:
    char* pointer;
};
