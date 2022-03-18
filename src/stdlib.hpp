#ifndef KERNEL_STDLIB
#define KERNEL_STDLIB

#include "stdint.h"
#include "string.hpp"

#define RAISE_ERROR(message) panic(message, __FILE__, __LINE__);

/* Read input untill Escape or Enter pressed, then returns an input string */
string ReadString();

/* Print 8-bit number as HEX */
void PrintByteAsString(uint8 byte);

/* Print 16-bit number as HEX */
void PrintShortAsString(uint16 word);

/* Print 32-bit number as HEX */
void PrintIntAsString(uint32 dword);

/* Print 32-bit number as usual decimal number */
void PrintNum(uint32 num);

/* Copy 'bytesAmount' bytes of memory from 'source' to 'destination' */
void CopyMem(uint8* source, uint8* destination, uint32 bytesAmount);

/* Compare two parts of memory */
bool CompareMem(uint8* first, uint8* seconds, uint32 bytesAmount);

/* Fill memory by given byte */
void FillMem(uint8* ptr, uint8 byte, uint32 amount);

/* Raises an error and halts the system */
void RaiseError(string message, const char* file, uint32 line);

#endif