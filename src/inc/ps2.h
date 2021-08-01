#ifndef C_PS2_DRIVER_LIB
#define C_PS2_DRIVER_LIB

#include "stdio.h"

extern uint8_t leftShiftPressed;
extern uint8_t leftCtrlPressed;
extern uint8_t capsLockActive;

/* Wait for key and returns its scancode. (Can ignore key releases) */
uint8_t ps2_scancode(uint8_t ignoreReleases);

/* Return pressed key in ASCII */
uint8_t ps2_readKey(void);

#endif
