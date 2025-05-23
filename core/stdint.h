#pragma once

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

// The compiler or IDE's IntelliSense must not use 64-bit compilers. 
#if __x86_64__ || _WIN64
#error No 64-bit support!
#endif

typedef unsigned int   size_t;
