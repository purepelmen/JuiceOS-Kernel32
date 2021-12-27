#pragma once

#define KERNEL_VERSION "1.0.3"

#include "stdlib.h"

/* Initalizes the kernel */
void InitializeKernel(void);

/* Kernel entry point */
void kernel_main(void);

/* Open console */
void OpenConsole(void);

/* Open OS menu */
void OpenMenu(void);

/* Open Memory dumper */
void OpenMemoryDumper(void);

/* Open system logs */
void OpenSystemLogs(void);

/* Open debug */
void OpenDebug(void);

/* Print new string to logs */
void PrintLog(uint8* str);
