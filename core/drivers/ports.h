#ifndef C_PORTS_LIB
#define C_PORTS_LIB

#include "../stdlib.h"

/* Read byte from port */
uint8 port_read8(uint16 port);

/* Write byte to port */
void port_write8(uint16 port, uint8 data);

/* Read word from port */
uint16 port_read16(uint16 port);

/* Write word to port */
void port_write16(uint16 port, uint16 data);

uint32 port_read32(uint16 port);

void port_write32(uint16 port, uint32 data);

#endif