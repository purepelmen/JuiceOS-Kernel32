#pragma once
#include "stdint.h"

enum class multiboot_tagtype : uint16
{
    NULL = 0,
    BOOT_COMMAND_LINE = 1,
    BOOTLOADER_NAME = 2,
    BASIC_MEMORY_INFO = 4,
    MEMORY_MAP = 6
};

enum class mb2_mmap_type : uint32
{
    AVAILABLE = 1,
    RESERVED = 2,
    APCI_RECLAIMABLE = 3,
    HIBERNATION_RESERVED = 4,
    DEFECTIVE_RAM_MODULE = 5,
};

struct multiboot_tag
{
    multiboot_tagtype type;
    uint16 flags;
    uint32 size;

    union
    {
        uint8 ptr;

        struct
        {
            uint32 memLower;
            uint32 memUpper;
        } __attribute__((packed)) basicMemInfo;

        struct
        {
            uint32 size;
            uint8 str[];
        } __attribute__((packed)) bootCommandLine;

        struct
        {
            uint32 entrySize;
            uint32 entryVersion;
            struct
            {
                unsigned long long baseAddr;
                unsigned long long length;
                mb2_mmap_type type;
                uint32 reserved;
            } __attribute__((packed)) entries[];
        } __attribute__((packed)) memoryMap;
    } __attribute__((packed)) data;

    bool is_end() { return type == multiboot_tagtype::NULL && size == 8; }
} __attribute__((packed));

inline const char* mb2_mmap_type_to_str(mb2_mmap_type type)
{
    switch (type)
    {
    case mb2_mmap_type::AVAILABLE: return "AVAILABLE";
    case mb2_mmap_type::RESERVED: return "RESERVED";
    case mb2_mmap_type::APCI_RECLAIMABLE: return "APCI_RECLAIMABLE";
    case mb2_mmap_type::HIBERNATION_RESERVED: return "HIBERNATION_RESERVED";
    case mb2_mmap_type::DEFECTIVE_RAM_MODULE: return "DEFECTIVE_RAM_MODULE";
    
    default: return nullptr;
    }
}

multiboot_tag* multiboot_traverse_tag(multiboot_tag* tag);
multiboot_tag* multiboot_find_tag(multiboot_tag* startTag, multiboot_tagtype type);
