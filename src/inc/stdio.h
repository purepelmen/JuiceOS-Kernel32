#ifndef C_STDIO_LIB
#define C_STDIO_LIB

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;
typedef unsigned char   boolean;

#define true                            1
#define false                           0

#define STANDART_SCREEN_COLOR           0x07
#define STANDART_INVERTED_SCREEN_COLOR  0x70
#define STANDART_CONSOLE_PREFIX_COLOR   0x02

#define PANIC(msg) panic(msg, __FILE__, __LINE__);

extern uint32   cursorX;
extern uint32   cursorY;
extern uint8    printColor;

/* Clear the screen */
void clear_screen(void);

/* Print given char and update scrolling/cursor */
void print_char(uint8 aChar);

/* Print given char. Do not update scrolling/cursor. Also don't handle newline and backspace */
void print_char_noupdates(uint8 aChar);

/* Print given string */
void print_string(const uint8* string);

/* Print given string using print_char_noupdates() function. */
void print_string_noupdates(const uint8* string);

/* Scrolling update function. If text is overflowing the screen, all screen content will be moved up */
void update_scrolling(void);

/* Cursor update function. Cursor moves to position that stores in cursorX and cursorY vars */
void update_cursor(void);

/* Enable the cursor. Parameters specifies size of cursor */
void enable_cursor(uint8 cursor_start, uint8 cursor_end);

/* Disable the cursor. */
void disable_cursor(void);

/* Getting input and writting it to a string. Enter or Escape breaks the input loop */
uint8* get_input();

/* Count the string length */
uint32 str_len(const uint8* str);

/* Compare the lengths of two strings */
boolean str_copmare_len(const uint8* str1, const uint8* str2);

/* Compare two strings */
boolean str_compare(const uint8* str1, const uint8* str2);

/* Make text uppercase */
void str_upper(const uint8* string, uint8* destination);

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

/* Fill memory by given byte */
void mem_set(uint8* ptr, uint8 byte, uint32 amount);

// Cause panic. It shows error message and halts the system.
void panic(const uint8* message, const char* file, uint32 line);

// Show an error.
void showError(const uint8* error, const uint8* causedBy, const uint8* handledBy, const uint8* details);

#endif