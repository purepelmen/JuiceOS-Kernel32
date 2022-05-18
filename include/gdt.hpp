#pragma once

#include "stdint.h"

struct gdt_descriptor
{
    uint16 limit_lower;
    uint16 base_lower;
    uint8 base_middle;
    uint8 access_flags;
    uint8 limit_high: 4;
    uint8 other_flags: 4;
    uint8 base_high;
} __attribute__((packed));

typedef struct gdt_descriptor gdt_desc_t;
