#pragma once
#include "stdint.h"
#include "string.h"

#include "console.h"

#define RAISE_ERROR(message) { raise_error_begin(message, __FILE__, __LINE__); raise_error_end(); }
#define RAISE_ERROR_D(message, desc, ...) { raise_error_begin(message, __FILE__, __LINE__); kconsole::printf(desc, __VA_ARGS__); raise_error_end(); }

/* Copy 'bytesAmount' bytes of memory from 'source' to 'destination' */
void mem_copy(void* source, void* destination, uint32 bytes_amount);

/* Compare two parts of memory */
bool mem_compare(uint8* first, uint8* seconds, uint32 bytes_amount);

/* Fill memory by given byte */
void mem_fill(void* ptr, uint8 byte, uint32 amount);

/* Convert UTF-16 string to ASCII string */
void utf16_to_ascii(uint8* buffer, uint16* utf16_str);

void strcpy(const char* source, char* dest);

void uint_to_hex(unsigned value, char* outBuffer, uint8 width);
void uint_to_str(unsigned value, char* outBuffer, int base = 10);
void int_to_str(int value, char* outBuffer, int base = 10);

void raise_error_begin(string message, const char* file, uint32 line);
void raise_error_end();
