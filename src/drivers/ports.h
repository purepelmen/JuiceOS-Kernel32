#ifndef C_PORTS_LIB
#define C_PORTS_LIB

#include "../stdio.h"

/* Read byte from port */
uint8 ReadPortByte(uint16 port);

/* Write byte to port */
void WritePortByte(uint16 port, uint8 data);

/* Read word from port */
uint16 ReadPortShort(uint16 port);

/* Write word to port */
void WritePortShort(uint16 port, uint16 data);

#endif