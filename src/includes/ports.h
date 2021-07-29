#ifndef C_PORTS_LIB
#define C_PORTS_LIB

/* Read byte from port */
uint8_t port_byte_in(uint16_t port);

/* Write byte to port */
void port_byte_out(uint16_t port, uint8_t data);

/* Read word from port */
uint16_t port_word_in(uint16_t port);

/* Write word to port */
void port_word_out(uint16_t port, uint16_t data);

#endif