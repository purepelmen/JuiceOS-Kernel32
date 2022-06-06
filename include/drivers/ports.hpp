#ifndef C_PORTS_LIB
#define C_PORTS_LIB

#include "stdlib.hpp"

/* Read byte from port */
uint8 port_byte_in(uint16 port);

/* Write byte to port */
void port_byte_out(uint16 port, uint8 data);

/* Read word from port */
uint16 port_word_in(uint16 port);

/* Write word to port */
void port_word_out(uint16 port, uint16 data);

uint32 port_dword_in(uint16 port);

void port_dword_out(uint16 port, uint32 data);

#endif