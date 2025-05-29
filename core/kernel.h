#pragma once

#define KERNEL_VERSION "1.0.3"

#include "string.h"

/* Print new string to logs */
void kernel_log(string str, ...);

string kernel_read_logs();
