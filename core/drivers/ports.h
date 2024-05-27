#pragma once
#include "../stdint.h"

/* Read byte from port */
inline uint8 port_read8(uint16 port)
{
    uint8 result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

/* Write byte to port */
inline void port_write8(uint16 port, uint8 data)
{
    __asm__ ("out %%al , %%dx" : :"a" (data), "d" (port));
}

/* Read word from port */
inline uint16 port_read16(uint16 port)
{
    uint16 result;
    __asm__ ("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

/* Write word to port */
inline void port_write16(uint16 port, uint16 data)
{
    __asm__ ("out %%ax, %%dx" : :"a" (data), "d" (port));
}

inline uint32 port_read32(uint16 port)
{
    uint32 result;
    __asm__ ("in %%dx, %%eax" : "=a" (result) : "d" (port));
    return result;
}

inline void port_write32(uint16 port, uint32 data)
{
    __asm__ ("out %%eax, %%dx" : :"a" (data), "d" (port));
}
