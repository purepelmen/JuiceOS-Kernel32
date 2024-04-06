#pragma once

#define KERNEL_VERSION "1.0.3"

#include "stdlib.hpp"
#include "string.hpp"

/* Open console */
void open_console(void);

/* Open OS menu */
void open_menu(void);

/* Open Memory dumper */
void open_memdumper(void);

/* Open system logs */
void open_syslogs(void);

/* Open debug */
void open_debugger(void);

/* Print new string to logs */
void kernel_print_log(string str);
