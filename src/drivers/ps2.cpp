#include <drivers/ports.hpp>
#include <drivers/ps2.hpp>

namespace kps2
{
    struct in_arguments inargs;

    const char* numbers_shifted = ")!@#$%^&*(";
    const char* ascii_table = "\x00\x1B" "1234567890-=" "\x08\x09" "qwertyuiop[]" 
                            "\x0A\x00" "asdfghjkl;'`" "\x00" "\\zxcvbnm,./" "\x00"
                            "*\x00" " " "\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00" 
                            "\x00\x00" "789" "-" "456" "+" "1230" 
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

    uint8 get_scancode(bool ignoreReleases)
    {
        while(true)
        {
            uint8 scan = port_byte_in(0x64);
            if(!(scan & 1))
            {
                asm volatile("hlt");
                continue;
            }

            if(ignoreReleases && (scan & (1 << 7)))
            {
                continue;
            } 
            else
            {
                break;
            }
        }

        uint8 result = port_byte_in(0x60);
        return result;
    }
    
    uint8 read_ascii()
    {
        uint8 _char;
        
        while(true)
        {
            uint8 scan = get_scancode(false);

            if(scan == 0x2A || scan == 0x36)
            {
                inargs.lshift = 1;
                continue;
            }
            else if(scan == 0xAA || scan == 0xB6)
            {
                inargs.lshift = 0;
                continue;
            }
            else if (scan == 0x1D)
            {
                inargs.lctrl = 1;
                continue;
            }
            else if(scan == 0x9D)
            {
                inargs.lctrl = 0;
                continue;
            }
            else if(scan & (1 << 7))
            {
                // Key release
                continue;
            }
            else if(scan == 0x3A)
            {
                inargs.capslock = !inargs.capslock;
                continue;
            }
            else if(scan == 0xFA)
            {
                // On command sent
                continue;
            }

            _char = ascii_table[scan];

            if(inargs.capslock == 0 && inargs.lshift == 0)
                break;

            if(_char >= 'a' && _char <= 'z')
            {
                _char -= 0x20;
                break;
            }

            if(_char >= '0' && _char <= '9')
            {
                _char -= 0x30;
                _char = numbers_shifted[_char];
                break;
            }

            if(_char == '=') 
                _char = '+';
            if(_char == '/') 
                _char = '?';
            if(_char == '-') 
                _char = '_';
            if(_char == '`') 
                _char = '~';
            if(_char == '[') 
                _char = '{';
            if(_char == ']') 
                _char = '}';
            if(_char == ';') 
                _char = ':';
            if(_char == '\'') 
                _char = '"';
            if(_char == ',') 
                _char = '<';
            if(_char == '.') 
                _char = '>';

            break;
        }

        return _char;
    }
}

// UP_ARROW = 48 (c8 - release)
// DOWN_ARROW = 50 (d0 - release)
// LEFT_ARROW = 4b (cb - release)
// RIGHT_ARROW = 4d (cd - release)
// CapsLock = 3a (ba release)
// ESCAPE = 01 (81 release)
// Right Shift = 36 (b6 release)
// Delete = 53 (d3 release)
