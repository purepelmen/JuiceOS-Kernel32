#include <drivers/ports.hpp>
#include <drivers/ps2.hpp>

const char* asciiTableNumsShifted = ")!@#$%^&*(";
const char* asciiTable = "\x00\x1B" "1234567890-=" "\x08\x09" "qwertyuiop[]" 
                         "\x0A\x00" "asdfghjkl;'`" "\x00" "\\zxcvbnm,./" "\x00"
                         "*\x00" " " "\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00" 
                         "\x00\x00" "789" "-" "456" "+" "1230" 
                         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

void Ps2::initialize()
{
    shiftPressed = false;
    leftCtrlPressed = false;
    capsLockEnabled = false;
}

uint8 Ps2::getCurrentKey()
{
    uint8 scan = ReadPortByte(0x64);

    if(scan & 1)
    {
        uint8 result = ReadPortByte(0x60);
        
        if(!(result & (1 << 7)))
            return result;
    }

    return 0;
}

uint8 Ps2::getScancode(bool ignoreReleases)
{
    while(1)
    {
        uint8 scan = ReadPortByte(0x64);
        if(!(scan & 1))
        {
            // asm volatile("hlt");
            continue;
        }

        if(ignoreReleases)
        {
            if(scan & (1 << 7)) 
                continue;
            else
                break;
        } 
        else
        {
            break;
        }
    }

    uint8 result = ReadPortByte(0x60);
    return result;
}

uint8 Ps2::readAscii()
{
    uint8 _char;
    
    while(1)
    {
        uint8 scan = getScancode(false);

        if(scan == 0x2A || scan == 0x36)
        {
            shiftPressed = 1;
            continue;
        }
        else if(scan == 0xAA || scan == 0xB6)
        {
            shiftPressed = 0;
            continue;
        }
        else if (scan == 0x1D)
        {
            leftCtrlPressed = 1;
            continue;
        }
        else if(scan == 0x9D)
        {
            leftCtrlPressed = 0;
            continue;
        }
        else if(scan & (1 << 7))
        {
            // Key release
            continue;
        }
        else if(scan == 0x3A)
        {
            capsLockEnabled = !capsLockEnabled;
            continue;
        }
        else if(scan == 0xFA)
        {
            // On command sent
            continue;
        }

        _char = asciiTable[scan];

        if(capsLockEnabled == 0 && shiftPressed == 0)
            break;

        if(_char >= 'a' && _char <= 'z')
        {
            _char -= 0x20;
            break;
        }

        if(_char >= '0' && _char <= '9')
        {
            _char -= 0x30;
            _char = asciiTableNumsShifted[_char];
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

void Ps2::operator>>(uint8& var)
{
    var = readAscii();
}

// UP_ARROW = 48 (c8 - release)
// DOWN_ARROW = 50 (d0 - release)
// LEFT_ARROW = 4b (cb - release)
// RIGHT_ARROW = 4d (cd - release)
// CapsLock = 3a (ba release)
// ESCAPE = 01 (81 release)
// Right Shift = 36 (b6 release)
// Delete = 53 (d3 release)
