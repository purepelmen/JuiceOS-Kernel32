#include "fat.h"
#include "mbr.h"

#include <drivers/screen.h>
#include <drivers/ide.h>
// #include <drivers/ahci.h>
#include <langutils.h>

#include <math.h>
#include <heap.h>
#include <console.h>
#include <mmanager.h>

using namespace kpart;

namespace kfat
{
    /// @brief An additional utility that parses useful entries of FAT directories.
    class EntryParser
    {
    private:
        bool parsingLongFilename = false;
        uint8 lfnChecksum;
    
    public:
        /// @brief Call for next found entry. Should be used for every type, except when the first filename char is 0x0.
        /// @param filenameBuff ASCII filename output buffer. Must be 256-byte sized at least, as both SFN or LFN may be copied to it.
        /// @return True if the valid filename is ready for this entry. (Doesn't return filenames for FATTR_DEVICE | FATTR_UNUSED)
        bool handle_next(dir_entry* entry, char* filenameBuff);
    };
    
    static uint8 calculate_checksum(const uint8* shortname);
    static bool is_valid_sector_size(size_t size);

    kstorage::FileSystem* probe(kstorage::BlockDevice* device)
    {
        char tempReadBuffer[512];

        device->read(0, 1, (uint16*)tempReadBuffer);
        fat_bpb* bpb = (fat_bpb*) tempReadBuffer;

        if(bpb->boot_sig != 0xAA55)
            return nullptr;

        if (mem_compare(bpb->oem_id, "EXFAT", 5) || mem_compare(bpb->oem_id, "NTFS", 4))
            return nullptr;

        if (!is_valid_sector_size(bpb->bytes_per_sector))
            return nullptr;
        if (!is_power_of_two(bpb->sectors_per_cluster))
            return nullptr;

        VolumeInfo info;
        info.bytesPerSector = bpb->bytes_per_sector;
        info.sectorsPerCluster = bpb->sectors_per_cluster;
        info.reservedSectors = bpb->reserved_sectors;
        info.sectorsPerFAT = bpb->sectors_per_fat;
        if (info.sectorsPerFAT == 0)
            info.sectorsPerFAT = bpb->ebpb_fat32.sectors_per_fat;
        info.rootDirEntries = bpb->root_dir_entries;
        info.fatCount = bpb->fat_count;

        if (info.reservedSectors == 0 || info.fatCount == 0 || info.sectorsPerFAT == 0)
            return nullptr;

        info.totalSectors = bpb->total_sectors;
        if(info.totalSectors == 0)
            info.totalSectors = bpb->large_sector_count;

        info.fatLBA = info.reservedSectors;
        info.rootDirectoryLBA = info.fatLBA + info.sectorsPerFAT * info.fatCount;
        info.rootDirSectors = divide_round_up<uint32>(info.rootDirEntries * sizeof(dir_entry), info.bytesPerSector);
        info.dataLBA = info.rootDirectoryLBA + info.rootDirSectors;

        uint32 dataSectors = info.totalSectors - (info.reservedSectors + info.sectorsPerFAT * info.fatCount + info.rootDirSectors);
        uint32 totalClusters = dataSectors / info.sectorsPerCluster;

        if (totalClusters == 0)
            info.type = FatType::ExFAT;
        else if (totalClusters < 4085)
            info.type = FatType::FAT12;
        else if (totalClusters < 65525)
            info.type = FatType::FAT16;
        else if (info.rootDirSectors == 0)
            info.type = FatType::FAT32;
        else
            return nullptr;

        if (info.type != FatType::FAT12 && info.type != FatType::FAT16 && info.type != FatType::FAT32)
        {
            kconsole::printf("[FAT] Non-FAT 12, 16 or 32 FS not supported\n");
            return nullptr;
        }

        uint8 signature = info.type == FatType::FAT32 ? bpb->ebpb_fat32.signature : bpb->ebpb.signature;
        if (signature != 0x28 && signature != 0x29)
            return nullptr;
            
        if (info.type == FatType::FAT32)
        {
            fat_fsinfo fsinfo;
            device->read(bpb->ebpb_fat32.fsinfo_sector, 1, (uint16*)&fsinfo);

            if (fsinfo.lead_signature != 0x41615252 || fsinfo.signature != 0x61417272 || fsinfo.trail_signature != 0xAA550000)
                return nullptr;
            
            info.rootDirectoryFile = 
            { 
                .inode = bpb->ebpb_fat32.root_dir_cluster, 
                .size = 0,
                .flags = FATTR_SUBDIRECTORY
            };
            info.chainEndCluster = 0x0FFFFFF7;

            mem_copy(bpb->ebpb_fat32.volume_label, info.bpbVolumeLabel, sizeof(bpb->ebpb_fat32.volume_label));
        }
        else
        {
            info.rootDirectoryFile = 
            { 
                .inode = 0, 
                .size = info.rootDirSectors,
                .flags = FATTR_SUBDIRECTORY
            };
            info.chainEndCluster = info.type == FatType::FAT12 ? 0xFF7 : 0xFFF7;
            
            mem_copy(bpb->ebpb.volume_label, info.bpbVolumeLabel, sizeof(bpb->ebpb.volume_label));
        }

        spaced_string_to_cstr(info.bpbVolumeLabel, 11);

        return kheap::create_new<FAT>(info);
    }
    
    void FAT::on_init()
    {
        size_t fatLength = volumeInfo.sectorsPerFAT * volumeInfo.bytesPerSector;
        loadedFATPageCount = kmmanager::get_page_count_for(fatLength);

        loadedFAT = (uint8*)kmmanager::alloc_pages(loadedFATPageCount);
        device->read(volumeInfo.fatLBA, fatLength / 512, (uint16*)loadedFAT);

        // Let's determine the volume name. Root dir has a FATTR_VOLUMELABEL entry, which is more prioritized than
        // 'volume_label' from BPB.
        if (!find_volumelabel(volumeName, sizeof(volumeName)))
        {
            strcpy(volumeInfo.bpbVolumeLabel, volumeName);
        }
    }

    const char* FAT::get_label()
    {
        return volumeName;
    }

    size_t FAT::get_size()
    {
        return volumeInfo.totalSectors * volumeInfo.bytesPerSector;
    }

    bool FAT::resolve_path(const char* path, kstorage::FileState& state)
    {
        kstorage::FileState rootDir = volumeInfo.rootDirectoryFile;
        char filename[kstorage::MAX_FILENAME_SIZE];

        uint32 parentCluster = rootDir.inode, parentDataLength = rootDir.size;
        kstorage::FileState last = rootDir;

        int pathIdx = 0;
        while (path[pathIdx] != 0x0)
        {
            if (!(last.flags & FATTR_SUBDIRECTORY))
            {
                kconsole::printf("Found unresolvable part: '%s' (accessing a file like a directory).\n", filename);
                return false;
            }

            for (; path[pathIdx] == '/'; pathIdx++);

            int i = 0;
            for (; path[pathIdx] != '/' && path[pathIdx] != 0x0; pathIdx++)
            {
                filename[i++] = path[pathIdx];
            }
            filename[i] = 0x0;

            if (filename[0] == 0x0)
                continue;

            kstorage::FileState resolved;
            if (!resolve_path_part(parentCluster, filename, resolved))
            {
                kconsole::printf("Found unresolvable part: '%s'.\n", filename);
                return false;
            }

            parentCluster = resolved.inode;
            parentDataLength = resolved.size;
            last = resolved;
        }

        state.inode = last.inode;
        state.size = last.size;
        state.flags = last.flags;
        state.position = 0;
        return true;
    }

    size_t FAT::read_file(kstorage::FileState& state, char* buffer, size_t length)
    {
        size_t initialPos = state.position;

        uint32 cluster = state.inode + (state.position / volumeInfo.bytesPerSector) / volumeInfo.sectorsPerCluster;
        ClusterReader reader{ this, cluster };
        do
        {
            reader.read_next();

            uint32 offset = state.position % reader.get_size();
            uint32 leftInCluster = min(reader.get_size() - offset, state.size - state.position);

            size_t copyPortion = min(leftInCluster, length);
            mem_copy(reader.as_buffer() + offset, buffer, copyPortion);

            state.position += copyPortion;
            length -= copyPortion;
        } 
        while(reader.has_more() && length > 0);

        return state.position - initialPos;
    }

    void FAT::read_dir(const char* path, kstorage::ReadDirCallback callback, void* context)
    {
        kstorage::FileState fileState;
        if (!resolve_path(path, fileState))
            return;

        if (!(fileState.flags & FATTR_SUBDIRECTORY))
        {
            kconsole::printf("Path '%s' is not a directory.\n", path);
            return;
        }
        
        kstorage::DirEntry currentEntry{};

        ClusterReader reader{ this, fileState.inode };
        EntryParser parser{};
        do
        {
            reader.read_next();

            // Parse every entry in the cluster
            for (int i = 0; i < reader.as_dir_count(); i++)
            {
                dir_entry* entry = reader.as_dir() + i;

                // Empty entry? This is the end.
                if (entry->dos_filename[0] == 0)
                    break;

                if (!parser.handle_next(entry, currentEntry.name))
                    continue;

                currentEntry.type = entry->attributes & FATTR_SUBDIRECTORY ? kstorage::DirEntryType::DIRECTORY : kstorage::DirEntryType::FILE;
                if (entry->attributes & FATTR_VOLUMELABEL)
                    currentEntry.type = kstorage::DirEntryType::SPECIAL;

                currentEntry.fileState =
                {
                    .inode = entry->first_cluster,
                    .size = entry->file_size,
                    .flags = entry->attributes,
                };

                if (!callback(context, currentEntry))
                    return;
            }
        }
        while (reader.has_more());
    }

    bool FAT::find_volumelabel(char* outLabel, size_t maxLabelSize)
    {
        bool found = false;
        auto check = [&](kstorage::DirEntry& entry)
        {
            if (entry.fileState.flags & FATTR_VOLUMELABEL)
            {
                strlcpy(entry.name, outLabel, maxLabelSize);
                found = true;
                return false;
            }

            return true;
        };
        
        read_dir("/", LambdaAdapter<decltype(check)>, &check);
        return found;
    }

    bool FAT::resolve_path_part(uint32 cluster, const char* part, kstorage::FileState& outResolved)
    {
        kconsole::printf("resolve_path_part() resolves '%s'...\n", part);
        char filenameBuff[kstorage::MAX_FILENAME_SIZE];
        
        ClusterReader reader{ this, cluster };
        EntryParser parser{};
        do
        {
            reader.read_next();

            // Parse every entry in the cluster
            for (int i = 0; i < reader.as_dir_count(); i++)
            {
                dir_entry* entry = reader.as_dir() + i;

                // Empty entry? This is the end.
                if (entry->dos_filename[0] == 0)
                    break;

                if (!parser.handle_next(entry, filenameBuff))
                    continue;

                if (string(part) == filenameBuff)
                {
                    outResolved.flags = entry->attributes;
                    outResolved.size = entry->file_size;
                    outResolved.inode = entry->first_cluster;
                    outResolved.position = 0;

                    return true;
                }
            }
        }
        while (reader.has_more());

        return false;
    }

    uint32 FAT::next_cluster(uint32 cluster)
    {
        if (volumeInfo.type == FatType::FAT12)
        {
            uint32 index = cluster + cluster / 2;

            uint16 value = *(uint16*)&loadedFAT[index];
            return cluster & 1 ? value >> 4 : value & 0xFFF;
        }
        else if (volumeInfo.type == FatType::FAT16)
        {
            uint16* fat = (uint16*)loadedFAT;
            return fat[cluster];
        }
        else
        {
            uint32* fat = (uint32*)loadedFAT;
            return fat[cluster] & 0x0FFFFFFF;
        }
    }

    void* FAT::load_sectors(uint32 lba, size_t sectors)
    {
        if (buffer != nullptr && sectors > lastAllocBufferSize)
        {
            kheap::free(buffer);
            buffer = nullptr;
        }
        
        if (buffer == nullptr)
        {
            buffer = kheap::alloc(sectors * volumeInfo.bytesPerSector);
            lastAllocBufferSize = sectors;
        }

        // kahci::read(dev_port, lba, 0, 1, (uint16*) buffer);

        // kide::AtaDevice device = kide::devices[dev_port];
        // kide::ata_read_sector(device.addr, device.isSlave, lba, 1, (uint16*)buffer);

        size_t sectorConverter = volumeInfo.bytesPerSector / 512;
        device->read(lba * sectorConverter, sectors * sectorConverter, (uint16*)buffer);
        return buffer;
    }

    uint8 calculate_checksum(const uint8* shortname)
    {
        uint8 checksum = 0;
        for(int i = 11; i; i--)
        {
            checksum = ((checksum & 1) ? 0x80 : 0) + (checksum >> 1) + *shortname++;
        }

        return checksum;
    }

    bool is_valid_sector_size(size_t size)
    {
        switch (size)
        {
        case 512:
        case 1024:
        case 2048:
        case 4096:
            return true;
        
        default:
            return false;
        }
    }

    bool EntryParser::handle_next(dir_entry* entry, char* filenameBuff)
    {
        // Erased entry? Skip
        if (entry->dos_filename[0] == 0xE5)
        {
            parsingLongFilename = false;
            return false;
        }
        
        // Long file name entry?
        if (entry->attributes == FATTR_LFN)
        {
            dir_lfn_entry* lfnEntry = (dir_lfn_entry*) entry;
            int seqNum = lfnEntry->seq_num & (~0x40);

            int lfnOffset = 13 * (seqNum - 1);
            lfnChecksum = lfnEntry->checksum;
            parsingLongFilename = true;
            
            uint16 lfnBuffer[13];
            mem_copy(lfnEntry->name_chars, lfnBuffer, 5 * 2);
            mem_copy(lfnEntry->name_chars2, lfnBuffer + 5, 6 * 2);
            mem_copy(lfnEntry->name_chars3, lfnBuffer + 11, 2 * 2);

            utf16_to_ascii(filenameBuff + lfnOffset, lfnBuffer, 13);
            return false;
        }

        if (entry->attributes & (FATTR_DEVICE | FATTR_UNUSED))
        {
            parsingLongFilename = false;
            return false;
        }

        bool useShortName = true;
        if (parsingLongFilename && lfnChecksum == calculate_checksum((uint8*)&entry->dos_filename))
        {
            useShortName = false;
            parsingLongFilename = false;
        }

        if (useShortName)
        {
            char* resultPtr = filenameBuff;

            strlcpy((char*)entry->dos_filename, resultPtr, 9);
            resultPtr = spaced_string_to_cstr(resultPtr, 8);
            
            if (!(entry->attributes & (FATTR_SUBDIRECTORY | FATTR_VOLUMELABEL)))
                resultPtr += strlcpy(".", resultPtr, 2);
            
            strlcpy((char*)entry->dos_fileext, resultPtr, 4);
            spaced_string_to_cstr(resultPtr, 3);
        }

        return true;
    }

    bool ClusterReader::read_next()
    {
        uint32 startSector;
        uint32 readCount;

        auto& volumeInfo = driver->get_volume_info();
        // Is this a root directory?
        if (cluster == 0)
        {
            startSector = volumeInfo.rootDirectoryLBA;
            readCount = volumeInfo.rootDirSectors;
        }
        else
        {
            startSector = volumeInfo.dataLBA + ((cluster - 2) * volumeInfo.sectorsPerCluster);
            readCount = volumeInfo.sectorsPerCluster;
        }

        buffer = driver->load_sectors(startSector, readCount);
        readBytes = readCount * volumeInfo.bytesPerSector;

        if (cluster == 0)
            return (hasMoreData = false);

        cluster = driver->next_cluster(cluster);
        return (hasMoreData = cluster < volumeInfo.chainEndCluster);
    }
}
