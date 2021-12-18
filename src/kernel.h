#pragma once

#define KERNEL_VERSION "1.0.3"

#include "stdio.h"

/* Initalize all descriptor tables, setup interrupts and so on... */
void InitializeKernel(void);

/* Kernel entry point */
void kernel_main(void);

/* Open console */
void OpenConsole(void);

/* Open OS menu */
void OpenMenu(void);

/* Open Memory dumper */
void OpenMemDumper(void);

/* Open system logs */
void OpenSysLogs(void);

/* Open debug */
void OpenDebug(void);

/* Print new string to logs */
void PrintLog(uint8* str);
