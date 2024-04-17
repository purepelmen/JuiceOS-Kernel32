#pragma once

#include "stdint.h"
#include "string.hpp"

#define RAISE_ERROR(message) raise_error(message, __FILE__, __LINE__);

/* Copy 'bytesAmount' bytes of memory from 'source' to 'destination' */
void mem_copy(uint8* source, uint8* destination, uint32 bytes_amount);

/* Compare two parts of memory */
bool mem_compare(uint8* first, uint8* seconds, uint32 bytes_amount);

/* Fill memory by given byte */
void mem_fill(uint8* ptr, uint8 byte, uint32 amount);

/* Convert UTF-16 string to ASCII string */
void utf16_to_ascii(uint8* buffer, uint16* utf16_str);

/* Raises an error and halts the system. */
void raise_error(string message, const char* file, uint32 line);
