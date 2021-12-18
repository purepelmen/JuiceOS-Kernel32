#ifndef C_PS2_DRIVER_LIB
#define C_PS2_DRIVER_LIB

#include "../stdio.h"

extern uint8 leftShiftPressed;
extern uint8 leftCtrlPressed;
extern uint8 capsLockActive;

/* Returns pressed key */
uint8 ps2_keyDown();

/* Wait for key and returns its scancode. (Can ignore key releases) */
uint8 ps2_scancode(uint8 ignoreReleases);

/* Return pressed key in ASCII */
uint8 ReadKey(void);

#endif
