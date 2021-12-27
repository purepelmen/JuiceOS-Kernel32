#ifndef C_PS2_DRIVER_LIB
#define C_PS2_DRIVER_LIB

#include "../stdlib.h"

extern uint8 leftShiftPressed;
extern uint8 leftCtrlPressed;
extern uint8 capsLockActive;

/* Returns pressed key */
uint8 Ps2KeyDown();

/* Wait for key and returns its scancode. (Can ignore key releases) */
uint8 Ps2GetScancode(uint8 ignoreReleases);

/* Return pressed key in ASCII */
uint8 Ps2ReadKey(void);

#endif
