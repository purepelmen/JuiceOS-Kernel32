#pragma once

#include "../stdlib.h"

namespace kps2
{
    struct in_arguments
    {
        bool capslock;
        bool lshift;
        bool lctrl;
        
        // If true, a new key data is set, and key_scancode and key_char contains valid fresh data that must be consumed.
        bool full_buffer;
        // Indicates that the current scancode means a pressed key (otherwise released) if the buffer is full.
        bool key_pressed;

        uint8 key_scancode;
        uint8 key_char;

        // Consume data as an ASCII char. This resets .full_buffer, because otherwise it won't be reset automatically by the driver.
        uint8 get_char();
        // Consume data as a scancode. This resets .full_buffer, because otherwise it won't be reset automatically by the driver.
        uint8 get_scancode();
    };

    extern struct in_arguments inargs;

    void init();
    
    // Read the next key scancode from the queue and wait if the buffer is empty. 
    uint8 get_scancode(bool ignoreReleases, bool discardQueue = true);
    // Read the next key as ASCII char from the queue and wait if the buffer is empty, ignoring key releases and other things.
    uint8 read_ascii(bool discardQueue = true);

    uint8 scancode_to_ascii(uint8 scancode, bool isCapitalized);
}
