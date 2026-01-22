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

    bool ISO9660::resolve_path(const char* path, kstorage::FileState& state)
    {
        auto rootDirEntry = volumeInfo.getRootDirEntry();
        char filename[kstorage::MAX_FILENAME_SIZE];

        uint32 parentLocationLBA = rootDirEntry->locationLBA, parentDataLength = rootDirEntry->dataLength;
        const iso9660_direntry* last = rootDirEntry;

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

            if (!(last->flags & 0x02))
            {
                kconsole::printf("Found unresolvable part: '%s' (accessing a file like a directory).\n", filename);
                return false;
            }

            iso9660_direntry* resolved = resolve_path_part(parentLocationLBA, parentDataLength, filename);
            if (resolved == nullptr)
            {
                kconsole::printf("Found unresolvable part: '%s'.\n", filename);
                return false;
            }

            parentLocationLBA = resolved->locationLBA;
            parentDataLength = resolved->dataLength;
            last = resolved;
        }

        state.inode = last->locationLBA;
        state.size = last->dataLength;
        state.flags = last->length;
        state.position = 0;
        return true;
    }

    size_t ISO9660::read(kstorage::FileState& state, char* buffer, size_t length)
    {
        size_t initialPos = state.position;
        while (length > 0 && (state.size - state.position) > 0)
        {
            device->read(CD_SECTOR(state.inode + (state.position / COMMON_LBA_SIZE)), 4, (uint16*)tempReadBuffer);
            
            size_t start = state.position % COMMON_LBA_SIZE;
            size_t leftToTheEndOfSector = COMMON_LBA_SIZE - start;

            size_t copyPortionSize = state.size - state.position;
            if (copyPortionSize > leftToTheEndOfSector)
                copyPortionSize = leftToTheEndOfSector;
            if (copyPortionSize > length)
                copyPortionSize = length;

            mem_copy(tempReadBuffer + start, buffer, copyPortionSize);
            buffer += copyPortionSize;

            state.position += copyPortionSize;
            length -= copyPortionSize;
        }

        return state.position - initialPos;
    }

    void ISO9660::read_dir(const char* path, kstorage::ReadDirCallback callback, void* context)
    {
        kstorage::FileState fileState;
        if (!resolve_path(path, fileState))
            return;

        if (!(fileState.flags & 0x02))
        {
            kconsole::printf("Path '%s' is not a directory.\n", path);
            return;
        }

        kstorage::DirEntry currentEntry{};

        int currSector = 0;
        while (currSector * COMMON_LBA_SIZE < fileState.size)
        {
            device->read(CD_SECTOR(fileState.inode + currSector), 4, (uint16*)tempReadBuffer);

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

                convert_iso9660_filename((char*) &dirEntry->filenameStartByte, dirEntry->filenameSize, resultFilename);
                if (string(part) == resultFilename)
                    return dirEntry;

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
