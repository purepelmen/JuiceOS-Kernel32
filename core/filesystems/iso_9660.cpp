#include "iso_9660.h"
#include <stdint.h>
#include <stdlib.h>

#include <kernel.h>
#include <heap.h>

#define PRIMARY_VOLUME_DESC_SECTOR 16
#define COMMON_LBA_SIZE 2048

#define CD_SECTOR(x) (x)*4

namespace kcd
{
    static char tempReadBuffer[2048];
    static void convert_iso9660_filename(const char* source, size_t sourceLength, char* outCString);

    kstorage::FileSystem* probe(kstorage::BlockDevice* device)
    {
        iso9660_volumedesc* volumeDesc = (iso9660_volumedesc*) tempReadBuffer;

        bool reachedTerminator = false;
        VolumeInfo volumeInfo{};

        int i = 0;
        while (i < 100)
        {
            device->read(CD_SECTOR(PRIMARY_VOLUME_DESC_SECTOR + i), 4, (uint16*)tempReadBuffer);

            if (!mem_compare(volumeDesc->identifier, "CD001", 5) || volumeDesc->version != 0x01)
                return nullptr;

            if (volumeDesc->type == iso9660_volumedesctype::VOLUME_DESC_SET_TERMINATOR)
            {
                reachedTerminator = true;
                break;
            }

            if (volumeDesc->type == iso9660_volumedesctype::PRIMARY_DESC_VOLUME)
            {
                if (volumeInfo.hasPVD)
                {
                    kernel_log("[kcd] WARNING: Found another PVD in the ISO 9660 filesystem.");
                    continue;
                }

                uint16 lbaSize = volumeDesc->data.primary_volume_desc.logicalBlockSize;
                if (lbaSize != COMMON_LBA_SIZE)
                {
                    kernel_log("[kcd] WARNING: PVD defined a non-common logical block size = %d. Driver won't work with this disk.\n", lbaSize);
                    return nullptr;
                }

                volumeInfo.setPrimaryVolumeDescLBA(PRIMARY_VOLUME_DESC_SECTOR + i);
                volumeInfo.setRootDirEntry((iso9660_direntry*)volumeDesc->data.primary_volume_desc.rootDirectoryEntry);
            }
            
            i++;
        }

        if (!reachedTerminator || !volumeInfo.hasPVD)
            return nullptr;

        return kheap::create_new<ISO9660>(volumeInfo);
    }

    void ISO9660::on_init()
    {
    }

    void ISO9660::read_dir(const char* path, kstorage::ReadDirCallback callback, void* context)
    {
        auto rootDirEntry = volumeInfo.getRootDirEntry();
        char filename[kstorage::MAX_FILENAME_SIZE];

        uint32 parentLocationLBA = rootDirEntry->locationLBA, parentDataLength = rootDirEntry->dataLength;

        int pathIdx = 0;
        while (path[pathIdx] != 0x0)
        {
            for (; path[pathIdx] == '/'; pathIdx++);

            int i = 0;
            for (; path[pathIdx] != '/' && path[pathIdx] != 0x0; pathIdx++)
            {
                filename[i++] = path[pathIdx];
            }
            filename[i] = 0x0;

            if (filename[0] == 0x0)
                continue;

            kcd::iso9660_direntry* resolved = resolve_path_part(parentLocationLBA, parentDataLength, filename);
            if (resolved == nullptr)
            {
                kconsole::printf("Found unresolvable part: '%s'.\n", filename);
                return;
            }

            parentLocationLBA = resolved->locationLBA;
            parentDataLength = resolved->dataLength;
        }

        kstorage::DirEntry currentEntry{};

        int currSector = 0;
        while (currSector * COMMON_LBA_SIZE < parentDataLength)
        {
            device->read(CD_SECTOR(parentLocationLBA + currSector), 4, (uint16*)tempReadBuffer);

            int i = 0;
            while (i < COMMON_LBA_SIZE)
            {
                auto dirEntry = (iso9660_direntry*)(tempReadBuffer + i);
                if (dirEntry->length == 0x0)
                    break;

                i += dirEntry->length;

                if (dirEntry->filenameStartByte == 0x0 || dirEntry->filenameStartByte == 0x01)
                    continue;
                
                convert_iso9660_filename((char*) &dirEntry->filenameStartByte, dirEntry->filenameSize, currentEntry.name);
                currentEntry.type = dirEntry->flags & 0x02 ? kstorage::DirEntryType::DIRECTORY : kstorage::DirEntryType::FILE;
                currentEntry.size = dirEntry->dataLength;

                if (!callback(context, currentEntry))
                    return;
            }

            currSector++;
        }
    }

    iso9660_direntry* ISO9660::resolve_path_part(uint32 parentLocationLBA, uint32 parentDataLength, const char* part)
    {
        kconsole::printf("resolve_path_part() resolves '%s'...\n", part);

        char resultFilename[kstorage::MAX_FILENAME_SIZE];

        int currSector = 0;
        while (currSector * COMMON_LBA_SIZE < parentDataLength)
        {
            device->read(CD_SECTOR(parentLocationLBA + currSector), 4, (uint16*)tempReadBuffer);

            int i = 0;
            while (i < COMMON_LBA_SIZE)
            {
                auto dirEntry = (iso9660_direntry*)(tempReadBuffer + i);
                if (dirEntry->length == 0x0)
                    break;

                if (dirEntry->flags & 0x02)
                {
                    convert_iso9660_filename((char*) &dirEntry->filenameStartByte, dirEntry->filenameSize, resultFilename);
                    if (string(part) == resultFilename)
                        return dirEntry;
                }

                i += dirEntry->length;
            }

            currSector++;
        }

        return nullptr;
    }
    
    void convert_iso9660_filename(const char* source, size_t sourceLength, char* outCString)
    {
        int limit = sourceLength - 1;
        for (; limit >= 0; limit--)
        {
            char ch = source[limit];
            if (ch != ' ')
                break;
        }

        int i = 0;
        for (; i <= limit; i++)
        {
            char ch = source[i];
            if (ch == ';')
                break;
            
            outCString[i] = ch;
        }
        
        outCString[i] = 0x0;
    }
}
