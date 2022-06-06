#pragma once

#include "stdint.hpp"
#include "string.hpp"

#define true  1
#define false 0

#define RAISE_ERROR(message) raise_error(message, __FILE__, __LINE__);

/* Read input untill Escape or Enter pressed, then returns an input string */
string read_string();

/* Print 8-bit number as HEX */
void print_hex_8bit(uint8 byte);

/* Print 16-bit number as HEX */
void print_hex_16bit(uint16 word);

/* Print 32-bit number as HEX */
void print_hex_32bit(uint32 dword);

/* Print 32-bit number as usual decimal number */
void print_number(uint32 num);

/* Print a formated string */
void printf(string str, ...);

/* Copy 'bytesAmount' bytes of memory from 'source' to 'destination' */
void mem_copy(uint8* source, uint8* destination, uint32 bytes_amount);

/* Compare two parts of memory */
bool mem_compare(uint8* first, uint8* seconds, uint32 bytes_amount);

/* Fill memory by given byte */
void mem_fill(uint8* ptr, uint8 byte, uint32 amount);

/* Raises an error and halts the system */
void raise_error(string message, const char* file, uint32 line);
