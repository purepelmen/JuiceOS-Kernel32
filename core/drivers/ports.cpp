#include "ports.h"

uint8 port_read8(uint16 port)
{
    uint8 result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void port_write8(uint16 port, uint8 data)
{
    __asm__ ("out %%al , %%dx" : :"a" (data), "d" (port));
}

uint16 port_read16(uint16 port)
{
    uint16 result;
    __asm__ ("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void port_write16(uint16 port, uint16 data)
{
    __asm__ ("out %%ax, %%dx" : :"a" (data), "d" (port));
}

uint32 port_read32(uint16 port)
{
    uint32 result;
    __asm__ ("in %%dx, %%eax" : "=a" (result) : "d" (port));
    return result;
}

void port_write32(uint16 port, uint32 data)
{
    __asm__ ("out %%eax, %%dx" : :"a" (data), "d" (port));
}
