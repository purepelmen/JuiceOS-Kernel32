#ifndef C_STDIO_LIB
#define C_STDIO_LIB

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;
typedef unsigned char   bool;

#define true                            1
#define false                           0

#define PANIC(msg) panic(msg, __FILE__, __LINE__);

/* Getting input and writting it to a string. Enter or Escape breaks the input loop */
uint8* get_input();

/* Count the string length */
uint32 str_len(const uint8* str);

/* Compare the lengths of two strings */
bool str_copmare_len(const uint8* _string1, const uint8* _string2);

/* Compare two strings */
bool str_compare(const uint8* _string1, const uint8* _string2);

/* Make text uppercase */
void str_upper(const uint8* original, uint8* destination);

/* Make text lowercase */
void str_lower(const uint8* string, uint8* destination);

/* Concat two strings */
void str_concat(uint8* concatTo, uint8* from);

/* Split string by separator */
uint8** str_split(const uint8* string, uint8 separator);

/* Print 8-bit number as HEX */
void print_hexb(uint8 byte);

/* Print 16-bit number as HEX */
void print_hexw(uint16 word);

/* Print 32-bit number as HEX */
void print_hexdw(uint32 dword);

/* Print 32-bit number as usual decimal number */
void print_number(uint32 num);

/* Copy 'bytesAmount' bytes of memory from 'source' to 'destination' */
void mem_copy(uint8* source, uint8* destination, uint32 bytesAmount);

/* Compare two parts of memory */
bool mem_cmp(uint8* first, uint8* seconds, uint32 bytesAmount);

/* Fill memory by given byte */
void mem_set(uint8* ptr, uint8 byte, uint32 amount);

// Cause panic. It shows error message and halts the system.
void panic(const uint8* message, const char* file, uint32 line);

// Show an error.
void showError(const uint8* error, const uint8* causedBy, const uint8* handledBy, const uint8* details);

#endif