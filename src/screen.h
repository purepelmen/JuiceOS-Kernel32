#ifndef C_SCREEN_DRIVER_LIB
#define C_SCREEN_DRIVER_LIB

#include "stdio.h"
#include "ports.h"

#define STANDART_SCREEN_COLOR           0x07
#define STANDART_INVERTED_SCREEN_COLOR  0x70
#define STANDART_CONSOLE_PREFIX_COLOR   0x02

extern uint32   cursorX;
extern uint32   cursorY;
extern uint8    printColor;

/* Enable the cursor. Parameters specifies size of cursor */
void enable_cursor(uint8 cursor_start, uint8 cursor_end);

/* Disable the cursor. */
void disable_cursor(void);

/* Cursor update function. Cursor moves to position that stores in cursorX and cursorY vars */
void update_cursor(void);

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

#endif