#ifndef C_STDIO_LIB
#define C_STDIO_LIB

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;
typedef unsigned char   bool;

#define true                            1
#define false                           0

#define PANIC(msg) panic(msg, __FILE__, __LINE__);

/* Read input untill Escape or Enter pressed, then returns an input string */
uint8* ReadString();

/* Returns string length */
uint32 GetStringLength(const uint8* str);

/* Compare the lengths of two strings */
bool CompareStringLength(const uint8* _string1, const uint8* _string2);

/* Compare two strings */
bool CompareString(const uint8* _string1, const uint8* _string2);

/* Converts string to upper case */
void ToUpperCase(const uint8* original, uint8* destination);

/* Converts string to lower case */
void ToLowerCase(const uint8* string, uint8* destination);

/* Concats two strings */
void ConcatString(uint8* concatTo, uint8* from);

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

// Cause panic. It shows error message and halts the system.
void Panic(const uint8* message, const char* file, uint32 line);

// Show an error.
void ShowError(const uint8* error, const uint8* causedBy, const uint8* handledBy, const uint8* details);

#endif