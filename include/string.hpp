#pragma once

#include "stdint.hpp"

class string
{
public:
    string(const char* char_sequence);
    string();
    
    uint32 length();

    bool compare(string other_str);
    bool compare_length(string other_str);

    void to_upper(string destination);
    void to_lower(string destination);

    void concat(string from);

    char operator[](int element);
    bool operator==(string str);

private:
    char* pointer;
};
