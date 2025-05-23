#pragma once
#include "stdint.h"

enum class multiboot_tagtype : uint16
{
    NULL = 0,
    BOOTLOADER_NAME = 2
};

struct multiboot_tag
{
    multiboot_tagtype type;
    uint16 flags;
    uint32 size;

    uint8 data;

    bool is_end() { return type == multiboot_tagtype::NULL && size == 8; }
} __attribute__((packed));

multiboot_tag* multiboot_traverse_tag(multiboot_tag* tag);
multiboot_tag* multiboot_find_tag(multiboot_tag* startTag, multiboot_tagtype type);
