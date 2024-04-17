#pragma once
#include "../stdlib.h"

namespace kparts
{
    struct mbr_partition
    {
        uint8 boot_flag;
        uint8 start_head;
        uint8 start_sector : 6;
        uint16 start_cylinder : 10;
        uint8 system_id;
        uint8 end_head;
        uint8 end_sector : 6;
        uint16 end_cylinder : 10;

        uint32 start_lba;
        uint32 lba_size;
    } __attribute__((packed)); 

    struct mbr
    {
        uint8 boot_code[440];
        uint32 disk_id;
        uint16 reserved;

        mbr_partition partitions[4];
        uint16 boot_sig;
    } __attribute__((packed));
}
