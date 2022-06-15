#include "filesystems/fat16.hpp"
#include "filesystems/mbr.hpp"

#include "drivers/screen.hpp"
#include "drivers/ahci.hpp"
#include "heap.hpp"

using namespace kparts;

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
            printf("[FAT16] Invalid boot signature\n");
            return false;
        }

        mbr_partition* sel_part = &mbr_s->partitions[partition];
        uint32 part_size = (sel_part->lba_size - sel_part->start_lba) * 512 / 1024;
        printf("[FAT16] Partition #%d: Size = %dKb\n", partition, part_size);

        partition_start = sel_part->start_lba;
        fat16_bpb* bpb = (fat16_bpb*) read(partition_start);
        if(bpb->signature != 0x28 && bpb->signature != 0x29)
        {
            printf("[FAT16] Invalid FAT16 signature\n");
            return false;
        }

        if(bpb->bytes_per_sector != 512)
        {
            printf("[FAT16] Allowed 512 bytes per sector FSes only\n");
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
        root_dir = fat_lba + sec_per_fat * fat_count;
        data_lba = root_dir + root_dir_entries * 32;

        printf("[FAT16] Init success\n");
        return true;
    }

    void FAT16::dir()
    {
        // Prepare LFN buffer and clear it with zeroes
        uint8 lfn_checksum = 0;
        if(lfn_buffer == nullptr)
        {
            lfn_buffer = (uint16*) malloc(512);
        }
        mem_fill((uint8*) lfn_buffer, 0, 256);

        printf("T     FILESIZE      FILENAME\n");
        printf("-----------------------------------------\n");

        int sector_i = 0;
        uint32 dir_sectors_count = root_dir_entries / 16;
        while(sector_i < dir_sectors_count)
        {
            dir_entry* curr_entry = (dir_entry*) read(root_dir + sector_i);

            int entry_i = 0;
            while(entry_i++ < 16)
            {
                // Empty entry? Skip
                if(curr_entry->dos_filename[0] == 0)
                {
                    curr_entry++;
                    continue;
                }

                // Erased entry? Skip
                if(curr_entry->dos_filename[0] == 0xE5)
                {
                    curr_entry++;
                    continue;
                }
                
                // Long file name entry?
                if(curr_entry->attributes & FATTR_LFN)
                {
                    dir_lfn_entry* lfn_entry = (dir_lfn_entry*) curr_entry;
                    int seq_num = lfn_entry->seq_num & (~0x40);

                    int lfn_buff_pos = 12 * (seq_num - 1);
                    lfn_checksum = lfn_entry->checksum;

                    for(int i = 0; i < 5; i++)
                        lfn_buffer[lfn_buff_pos++] = lfn_entry->name_chars[i];

                    for(int i = 0; i < 6; i++)
                        lfn_buffer[lfn_buff_pos++] = lfn_entry->name_chars2[i];

                    for(int i = 0; i < 2; i++)
                        lfn_buffer[lfn_buff_pos++] = lfn_entry->name_chars3[i];
                    
                    curr_entry++;
                    continue;
                }

                if(curr_entry->attributes & (FATTR_HIDDEN | FATTR_VOLUMELABEL | FATTR_DEVICE | FATTR_UNUSED))
                {
                    curr_entry++;
                    continue;
                }

                static uint8 fn_buff[256];
                if(lfn_checksum != 0)
                {
                    if(lfn_checksum != calc_checksum((uint8*) &curr_entry->dos_filename))
                    {
                        printf("[FAT16] Invalid checksum\n");
                        curr_entry++;
                        continue;
                    }

                    utf16_to_ascii(fn_buff, lfn_buffer);
                    lfn_checksum = 0;
                }
                else
                {
                    int buff_pos = 0;
                    for(int i = 0; i < 8 && curr_entry->dos_filename[i] != ' '; i++)
                    {
                        fn_buff[buff_pos++] = curr_entry->dos_filename[i];
                    }

                    fn_buff[buff_pos++] = '.';

                    for(int i = 0; i < 3; i++)
                    {
                        fn_buff[buff_pos++] = curr_entry->dos_fileext[i];
                    }

                    fn_buff[buff_pos] = 0x0;
                }

                string type_str = curr_entry->attributes & FATTR_SUBDIRECTORY ? "D" : "F";

                printf("[%s]   %dB", type_str, curr_entry->file_size);
                kscreen::outargs.cursor_x = 20;
                printf("%s\n", fn_buff);
                
                curr_entry++;
            }

            sector_i++;
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
            buffer = (uint8*) malloc(513);
            if((uint32) buffer & 1)
                buffer++;
        }

        kahci::read(dev_port, lba, 0, 1, (uint16*) buffer);
        return buffer;
    }
}
