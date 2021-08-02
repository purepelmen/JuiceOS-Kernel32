#ifndef C_STDIO_LIB
#define C_STDIO_LIB

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define true 1
#define false 0

#define STANDART_SCREEN_COLOR 0x07
#define STANDART_INVERTED_SCREEN_COLOR 0x70
#define STANDART_CONSOLE_PREFIX_COLOR 0x02

extern uint32_t cursorX;
extern uint32_t cursorY;
extern uint8_t printColor;
extern uint8_t inputExitCode;

/* Clear the screen */
void clear_screen(void);

/* Print given char and update scrolling/cursor */
void print_char(uint8_t aChar);

/* Print given char. Do not update scrolling/cursor. Also don't handle newline and backspace */
void print_char_noupdates(uint8_t aChar);

/* Print given string */
void print_string(const uint8_t* string);

/* Print given string using print_char_noupdates() function. */
void print_string_noupdates(const uint8_t* string);

/* Scrolling update function. If text is overflowing the screen, all screen content will be moved up */
void update_scrolling(void);

/* Cursor update function. Cursor moves to position that stores in cursorX and cursorY vars */
void update_cursor(void);

/* Enable the cursor. Parameters specifies size of cursor */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);

/* Disable the cursor. */
void disable_cursor(void);

/* Getting input and writting it to a string. Enter or Escape breaks the input loop */
uint8_t* get_input();

/* Count the string length */
uint32_t str_len(const uint8_t* str);

/* Compare the lengths of two strings */
uint8_t str_copmare_len(const uint8_t* str1, const uint8_t* str2);

/* Compare two strings */
uint8_t str_compare(const uint8_t* str1, const uint8_t* str2);

/* Make text uppercase */
void str_upper(const uint8_t* string, uint8_t* destination);

/* Make text lowercase */
void str_lower(const uint8_t* string, uint8_t* destination);

/* Concat two strings */
void str_concat(uint8_t* concatTo, uint8_t* from);

/* Split string by separator */
void str_split(const uint8_t* string, uint8_t* destination, uint8_t separator, uint32_t index);

/* Print 8-bit number as HEX */
void print_hexb(uint8_t byte);

/* Print 16-bit number as HEX */
void print_hexw(uint16_t word);

/* Print 32-bit number as HEX */
void print_hexdw(uint32_t dword);

/* Copy 'bytesAmount' bytes of memory from 'source' to 'destination' */
void mem_copy(uint8_t* source, uint8_t* destination, uint32_t bytesAmount);

/* Fill memory by given byte */
void mem_set(uint8_t* ptr, uint8_t byte, uint32_t amount);

#define PANIC(msg) panic(msg, __FILE__, __LINE__);

void panic(const uint8_t* message, const char *file, uint32_t line);

#endif