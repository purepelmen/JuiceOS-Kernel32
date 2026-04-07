#pragma once
#include "stdint.h"

namespace kpaging
{
    struct directory_entry
    {
        uint8 present: 1;
        uint8 writable: 1;
        uint8 usermode_accessable: 1;
        uint8 write_through_caching: 1;
        uint8 caching_disabled: 1;
        uint8 was_accessed: 1;
        uint8 is_dirty: 1;
        uint8 four_mb_pages: 1;

        union
        {
            struct
            {
                uint8 ignored: 4;
                uint32 address: 20;
            } __attribute__((packed)) to_pagetable;
            
            struct
            {
                uint8 global: 1;
                uint8 ignored: 3;
                uint8 pat: 1;
                uint32 address_39_32 : 8;
                uint8 reserved: 1;
                uint32 address_31_22 : 10;
            } __attribute__((packed)) to_4MB;
        } __attribute__((packed));
        
        // uint8 global: 1;
        // uint8 ignored: 3;
        // uint32 address: 20;
    } __attribute__((packed));

    struct table_entry
    {
        uint8 present: 1;
        uint8 read_write: 1;
        uint8 user_mode: 1;
        uint8 write_through_caching: 1;
        uint8 cache_disabled: 1;
        uint8 accessed: 1;
        uint8 dirty: 1;
        uint8 pat: 1;
        uint8 global: 1;
        uint8 ignored: 3;
        uint32 address : 20;
    } __attribute__((packed));;

    typedef struct directory_entry dir_entry_t;

    struct directory
    {
        directory_entry entries[1024];
    } __attribute__((packed));

    struct page_table
    {
        table_entry entries[1024];
    } __attribute__((packed));

    struct addr
    {
        uint32 offset     : 12,
               tableEntry : 10,
               dirEntry   : 10; 

        static addr from(void* ptr) { return (addr&)ptr; } 
    } __attribute__((packed));
    
    void init(directory* directory);
    void map_address(uint32 address);
}
