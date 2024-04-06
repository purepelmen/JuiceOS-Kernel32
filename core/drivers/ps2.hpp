#pragma once

#include "../stdlib.hpp"

namespace kps2
{
    struct in_arguments
    {
        bool capslock;
        bool lshift;
        bool lctrl;

        bool key_pressed;
        bool full_buffer;

        uint8 key_scancode;
        uint8 key_char;

        uint8 get_char();
        uint8 get_scancode();
    };

    extern struct in_arguments inargs;

    void init();
    uint8 get_scancode(bool ignoreReleases);
    uint8 read_ascii();
}
