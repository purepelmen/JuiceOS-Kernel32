#include "screen.hpp"
#include "ports.hpp"
#include "ps2.hpp"
#include "../kernel.hpp"
#include "../isr.hpp"

namespace kps2
{
    struct in_arguments inargs;

    const char* numbers_shifted = ")!@#$%^&*(";
    const char* ascii_table = "\x00\x1B" "1234567890-=" "\x08\x09" "qwertyuiop[]" 
                            "\x0A\x00" "asdfghjkl;'`" "\x00" "\\zxcvbnm,./" "\x00"
                            "*\x00" " " "\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00" 
                            "\x00\x00" "789" "-" "456" "+" "1230" 
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

    static void keyboard_handler(kisr::regs_t regs);
    static uint8 scancode_to_ascii(uint8 scancode);

    uint8 in_arguments::get_char()
    {
        full_buffer = false;
        return key_char;
    }

    uint8 in_arguments::get_scancode()
    {
        full_buffer = false;
        return key_scancode;
    }

    void init()
    {
        kisr::register_handler(IRQ_BASE + 1, keyboard_handler);
        kernel_print_log("Keyboard driver init completed.\n");
    }

    uint8 get_scancode(bool ignoreReleases)
    {
        while(true)
        {
            asm volatile("hlt");

            if(inargs.full_buffer == false)
                continue;

            uint8 scancode = inargs.get_scancode();
            if(ignoreReleases && (scancode & (1 << 7)))
            {
                continue;
            }

            return scancode;
        }
    }
    
    uint8 read_ascii()
    {
        while(true)
        {
            asm volatile("hlt");

            if(inargs.full_buffer == false)
                continue;

            if(inargs.key_pressed == false)
                continue;

            return inargs.get_char();
        }
    }

    static void keyboard_handler(kisr::regs_t regs)
    {
        uint8 result = port_byte_in(0x60);

        inargs.key_pressed = !(result & (1 << 7));
        inargs.full_buffer = true;

        inargs.key_scancode = result;

        if(result == 0x2A || result == 0x36)
        {
            inargs.lshift = true;
            return;
        }
        else if(result == 0xAA || result == 0xB6)
        {
            inargs.lshift = false;
            return;
        }
        else if (result == 0x1D)
        {
            inargs.lctrl = true;
            return;
        }
        else if(result == 0x9D)
        {
            inargs.lctrl = false;
            return;
        }
        else if(result == 0x3A)
        {
            inargs.capslock = !inargs.capslock;
            return;
        }

        inargs.key_char = scancode_to_ascii(result);
    }

    static uint8 scancode_to_ascii(uint8 scancode)
    {
        if(scancode & (1 << 7))
        {
            // Key release
            return 0;
        }
        else if(scancode == 0xFA)
        {
            // On command sent
            return 0;
        }

        uint8 ascii = ascii_table[scancode];

        if(inargs.capslock == 0 && inargs.lshift == 0)
            return ascii;

        if(ascii >= 'a' && ascii <= 'z')
        {
            ascii -= 0x20;
            return ascii;
        }

        if(ascii >= '0' && ascii <= '9')
        {
            ascii -= 0x30;
            ascii = numbers_shifted[ascii];

            return ascii;
        }

        if(ascii == '=') 
            ascii = '+';
        if(ascii == '/') 
            ascii = '?';
        if(ascii == '-') 
            ascii = '_';
        if(ascii == '`') 
            ascii = '~';
        if(ascii == '[') 
            ascii = '{';
        if(ascii == ']') 
            ascii = '}';
        if(ascii == ';') 
            ascii = ':';
        if(ascii == '\'') 
            ascii = '"';
        if(ascii == ',') 
            ascii = '<';
        if(ascii == '.') 
            ascii = '>';

        return ascii;
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
