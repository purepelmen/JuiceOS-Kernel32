#pragma once

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

#if __x86_64__ || _WIN64
#error No 64-bit support!
#endif

typedef unsigned int   size_t;
