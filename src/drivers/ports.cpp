/*
    *
    **
    *** PORTS.C -- provides functions for I/O operations with ports.
    **
    *
*/

#include "drivers/ports.hpp"

uint8 ReadPortByte(uint16 port)
{
    uint8 result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void WritePortByte(uint16 port, uint8 data)
{
    __asm__ ("out %%al , %%dx" : :"a" (data), "d" (port));
}

uint16 ReadPortShort(uint16 port)
{
    uint16 result;
    __asm__ ("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void WritePortShort(uint16 port, uint16 data)
{
    __asm__ ("out %%ax, %%dx" : :"a" (data), "d" (port));
}