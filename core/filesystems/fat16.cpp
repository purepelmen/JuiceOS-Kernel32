#include "fat16.h"
#include "mbr.h"

#include <drivers/screen.h>
#include <drivers/ahci.h>

#include <heap.h>
#include <console.h>

using namespace kpart;

namespace kfat
{
    FAT16::FAT16(uint8 dev_port)
    {
        this->dev_port = dev_port;
    }

    bool FAT16::init(uint8 partition)
    {
        mbr* mbr_s = (mbr*) read(0x0);
        if(mbr_s->boot_sig != 0xAA55)
        {
            kconsole::printf("[FAT16] Invalid boot signature\n");
            return false;
        }

        mbr_partition* sel_part = &mbr_s->partitions[partition];
        uint32 part_size = (sel_part->lba_size - sel_part->start_lba) * 512 / 1024;
        kconsole::printf("[FAT16] Partition #%d: Size = %dKb\n", partition, part_size);

        partition_start = sel_part->start_lba;
        fat16_bpb* bpb = (fat16_bpb*) read(partition_start);
        if(bpb->signature != 0x28 && bpb->signature != 0x29)
        {
            kconsole::printf("[FAT16] Invalid FAT16 signature\n");
            return false;
        }

        if(bpb->bytes_per_sector != 512)
        {
            kconsole::printf("[FAT16] Allowed 512 bytes per sector FSes only\n");
            return false;
        }

        sec_per_cluster = bpb->sectors_per_cluster;
        reserved_secs = bpb->reserved_sectors;
        sec_per_fat = bpb->sectors_per_fat;

        root_dir_entries = bpb->root_dir_entries;
        fat_count = bpb->fat_count;

        total_sects = bpb->total_sectors;
        if(total_sects == 0)
        {
            total_sects = bpb->large_sector_count;
        }

        fat_lba = partition_start + reserved_secs;
        root_dir_lba = fat_lba + sec_per_fat * fat_count;
        data_lba = root_dir_lba + root_dir_entries / 16;

        uint32 data_sectors = total_sects - (reserved_secs + fat_count * sec_per_fat + root_dir_entries / 26);
        uint32 total_clusters = data_sectors / sec_per_cluster;

        if(total_clusters < 4085)
        {
            kconsole::printf("[FAT16] FAT12 FSes not supported\n");
            return false;
        }

        kconsole::printf("[FAT16] Init success\n");
        return true;
    }

    void FAT16::dir(uint16 cluster)
    {
        // Prepare LFN buffer and clear it with zeroes
        uint8 lfn_checksum = 0;
        if(lfn_buffer == nullptr)
        {
            lfn_buffer = (uint16*) kheap::alloc(512);
        }
        mem_fill((uint8*) lfn_buffer, 0, 512);

        kconsole::printf("T     FILESIZE      FILENAME\n");
        kconsole::printf("-----------------------------------------\n");

        // Parse all clusters
        while(true)
        {
            uint32 start_sector;
            uint32 sectors_read_count;

            if(cluster == 0)
            {
                // Parse root dir
                start_sector = root_dir_lba;
                sectors_read_count = root_dir_entries / 16;
            }
            else
            {
                // Parse desired cluster
                start_sector = data_lba + ((cluster - 2) * sec_per_cluster);
                sectors_read_count = sec_per_cluster;
            }

            // Parse every sector in the cluster
            for(int sector_ind = 0; sector_ind < sectors_read_count; sector_ind++)
            {
                // Read sector
                dir_entry* entry = (dir_entry*) read(start_sector + sector_ind);

                // Parse every entry of the sector
                int entry_ind = 0;
                while(entry_ind++ < 16)
                {
                    // Empty entry? Skip
                    if(entry->dos_filename[0] == 0)
                    {
                        entry++;
                        continue;
                    }

                    // Erased entry? Skip
                    if(entry->dos_filename[0] == 0xE5)
                    {
                        entry++;
                        continue;
                    }
                    
                    // Long file name entry?
                    if(entry->attributes & FATTR_LFN)
                    {
                        dir_lfn_entry* lfn_entry = (dir_lfn_entry*) entry;
                        int seq_num = lfn_entry->seq_num & (~0x40);

                        int lfn_buff_pos = 12 * (seq_num - 1);
                        lfn_checksum = lfn_entry->checksum;

                        for(int i = 0; i < 5; i++)
                            lfn_buffer[lfn_buff_pos++] = lfn_entry->name_chars[i];

                        for(int i = 0; i < 6; i++)
                            lfn_buffer[lfn_buff_pos++] = lfn_entry->name_chars2[i];

                        for(int i = 0; i < 2; i++)
                            lfn_buffer[lfn_buff_pos++] = lfn_entry->name_chars3[i];
                        
                        entry++;
                        continue;
                    }

                    if(entry->attributes & (FATTR_HIDDEN | FATTR_VOLUMELABEL | FATTR_DEVICE | FATTR_UNUSED))
                    {
                        entry++;
                        continue;
                    }

                    static uint8 fn_buff[256];
                    if(lfn_checksum != 0)
                    {
                        if(lfn_checksum != calc_checksum((uint8*) &entry->dos_filename))
                        {
                            kconsole::printf("[FAT16] Invalid checksum\n");
                            entry++;
                            continue;
                        }

                        utf16_to_ascii(fn_buff, lfn_buffer);
                        lfn_checksum = 0;
                    }
                    else
                    {
                        int buff_pos = 0;
                        for(int i = 0; i < 8 && entry->dos_filename[i] != ' '; i++)
                        {
                            fn_buff[buff_pos++] = entry->dos_filename[i];
                        }

                        fn_buff[buff_pos++] = '.';

                        for(int i = 0; i < 3; i++)
                        {
                            fn_buff[buff_pos++] = entry->dos_fileext[i];
                        }

                        fn_buff[buff_pos] = 0x0;
                    }

                    string type_str = entry->attributes & FATTR_SUBDIRECTORY ? "D" : "F";

                    kconsole::printf("[%s]   %dB %d", type_str, entry->file_size, entry->first_cluster);
                    kscreen::outargs.cursor_x = 20;
                    kconsole::printf("%s\n", fn_buff);
                    
                    entry++;
                }
            }

            // It was root dir parsing? Stop
            if(cluster == 0)
                break;

            // Calulcate offset in FAT and the sector of the FAT
            uint32 entry_offset = (cluster * 2) / 512;
            uint16* fat = (uint16*) read(fat_lba + (cluster * 2 / 512));

            // Get new cluster from the FAT
            cluster = fat[cluster];

            // Is it the last entry in a chain or bad cluster?
            if(cluster >= 0xFFF8 || cluster == 0xFFF7)
                break;
        }
    }

    uint8 FAT16::calc_checksum(const uint8* shortname)
    {
        uint8 checksum = 0;
        for(int i = 11; i; i--)
        {
            checksum = ((checksum & 1) ? 0x80 : 0) + (checksum >> 1) + *shortname++;
        }

        return checksum;
    }

    uint8* FAT16::read(uint32 lba)
    {
        // Allocate buffer for one sector
        if(buffer == nullptr)
        {
            buffer = (uint8*) kheap::alloc(513);
            if((uint32) buffer & 1)
                buffer++;
        }

        kahci::read(dev_port, lba, 0, 1, (uint16*) buffer);
        return buffer;
    }
}
