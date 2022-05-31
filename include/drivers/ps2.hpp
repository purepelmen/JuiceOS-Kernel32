#pragma once

#include "stdlib.hpp"

namespace kps2
{
    struct in_arguments
    {
        bool capslock;
        bool lshift;
        bool lctrl;
    };

    extern struct in_arguments inargs;

    uint8 get_scancode(bool ignoreReleases);
    uint8 read_ascii();
}
