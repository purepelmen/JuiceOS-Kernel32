#include "iso_9660.h"
#include <stdint.h>
#include <stdlib.h>

#include <kernel.h>
#include <heap.h>
#include <math.h>

#define PRIMARY_VOLUME_DESC_SECTOR 16
#define COMMON_LBA_SIZE 2048

#define CD_SECTOR(x) (x)*4

namespace kcd
{
    static char tempReadBuffer[COMMON_LBA_SIZE * 1];
    static char tempSuspTraverserReadBuffer[COMMON_LBA_SIZE * 1];

    static void convert_iso9660_filename(const char* source, size_t sourceLength, char* outCString);
    static int retrieve_rockridge_filename(ISO9660* driver, iso9660_direntry* entry, char* filenameBuff, size_t outBuffMaxLength);

    typedef bool (*SUSPTraverser)(void* context, susp_tag* tag);
    static void traverse_susp_tags(ISO9660* driver, uint8* currentSuspTag, uint8* suaEnd, void* context, SUSPTraverser traverser);

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
        check_susp_support();
        check_rockridge_support();

        if (supportsSusp)
            kernel_log("SUSP support detected.\n");
        if (supportsSusp)
            kernel_log("RockRidge support detected.\n");

        // This is just an example for now.
        // {
        //     auto rootDirEntry = volumeInfo.getRootDirEntry();

        //     kconsole::print("Reading main file...\n");
        //     iso9660_direntry* osFile = resolve_path_part(rootDirEntry->locationLBA, rootDirEntry->dataLength, "juiceos_.elf");

        //     char filenameBuff[kstorage::MAX_FILENAME_SIZE];
        //     retrieve_rockridge_filename(this, osFile, filenameBuff, 256);

        //     kconsole::printf("The fullname: %s\n", filenameBuff);
        // }
    }

    void ISO9660::check_susp_support()
    {
        auto rootDirEntry = volumeInfo.getRootDirEntry();
        iso9660_direntry* thisDir = resolve_path_part(rootDirEntry->locationLBA, rootDirEntry->dataLength, ".");
   
        // Check only for the first tag.
        traverse_susp_tags(this, thisDir->getSua(), thisDir->getSuaEnd(), this, [](void* context, susp_tag* tag) 
        {
            ISO9660* driver = (ISO9660*)context;
            if (mem_compare(tag->name, "SP", 2) && mem_compare(&tag->content.nextByte, "\xBE\xEF", 2) && tag->version == 1)
                driver->supportsSusp = true;

            return false;
        });
    }

    void ISO9660::check_rockridge_support()
    {
        if (!supportsSusp)
            return;
        
        auto rootDirEntry = volumeInfo.getRootDirEntry();
        iso9660_direntry* thisDir = resolve_path_part(rootDirEntry->locationLBA, rootDirEntry->dataLength, ".");
   
        // Check only for the first tag.
        traverse_susp_tags(this, thisDir->getSua(), thisDir->getSuaEnd(), this, [](void* context, susp_tag* tag) 
        {
            ISO9660* driver = (ISO9660*)context;
            if (mem_compare(tag->name, "ER", 2))
            {
                auto& tagER = tag->content.tag_ER;
            
                char extID[16+1];
                int clampedSize = min<uint8>(16, tagER.extIDLength);
                mem_copy(tag->content.tag_ER.restStrings, extID, clampedSize);
                extID[clampedSize] = 0x0;

                kernel_log("[kcd] (SUSP) Found ER field: '%s'\n", extID);

                // Check for Rock Ridge 1.12, Rock Ridge 1.09-1.10, and unknown RockRidge version respectively.
                string compared{ extID };
                if (compared == "IEEE_P1282" || compared == "RRIP 1991A" || compared == "IEEE_1282")
                    driver->supportsRockRidge = true;

                return false;
            }
            else if (mem_compare(tag->name, "RR", 2))
            {
                // 'RR' may be used by older versions of RockRidge.
                // Not sure if it has some paramaters.
                driver->supportsRockRidge = true;
                return false;
            }

            return true;
        });
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
        state.flags = last->flags;
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

        char filenameBuff[kstorage::MAX_FILENAME_SIZE];

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

                convert_iso9660_filename((char*) &dirEntry->filenameStartByte, dirEntry->filenameSize, filenameBuff);
                if (string(part) == filenameBuff)
                    return dirEntry;

                i += dirEntry->length;
            }

            currSector++;
        }

        return nullptr;
    }
    
    void convert_iso9660_filename(const char* source, size_t sourceLength, char* outCString)
    {
        if (source[0] == 0x00 || source[0] == 0x01)
        {
            strcpy(source[0] ? ".." : ".", outCString);
            return;
        }

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

    int retrieve_rockridge_filename(ISO9660* driver, iso9660_direntry* entry, char* filenameBuff, size_t outBuffMaxLength)
    {
        struct cback_data
        {
            char* filenameBuff;
            size_t written = 0;

            size_t maxLimit = 0;
            bool isFailed = false;
        } tempData;

        tempData.filenameBuff = filenameBuff;
        tempData.maxLimit = min(outBuffMaxLength, kstorage::MAX_FILENAME_SIZE);
        tempData.maxLimit--;  // For null-terminator.

        if (tempData.maxLimit < 1)
            return 0;

        traverse_susp_tags(driver, entry->getSua(), entry->getSuaEnd(), &tempData, [](void* context, susp_tag* tag) 
        {
            cback_data* data = (cback_data*)context;
            if (!mem_compare(tag->name, "NM", 2))
                return true;

            auto& tagNM = tag->content.tag_NM;
            size_t strPortionLen = tag->length - 5;

            if (data->written + strPortionLen > data->maxLimit)
            {
                kernel_log("[kcd] ERROR: During reading a direntry (w/ Rockridge support) detected too big str: accumulated len=%d\n");
                data->isFailed = true;
                return false;
            }

            mem_copy(tagNM.content, &data->filenameBuff[data->written], strPortionLen);
            data->written += strPortionLen;
            
            // Continue only if the field has such flag.
            return (tagNM.flags & susp_NM_flags_CONTINUE) != 0;
        });

        filenameBuff[tempData.written++] = 0x0;
        return tempData.isFailed ? -1 : tempData.written;
    }
    
    void traverse_susp_tags(ISO9660* driver, uint8* currentSuspTag, uint8* suaEnd, void* context, SUSPTraverser traverser)
    {
        while (currentSuspTag + ((uint32)currentSuspTag % 2) < suaEnd)
        {
            susp_tag* tag = (susp_tag*)currentSuspTag;

            if (mem_compare(tag->name, "CE", 2))
            {
                auto tagCE = tag->content.tag_CE;
                driver->get_device()->read(CD_SECTOR(tagCE.continuationLBA), 4, (uint16*)tempSuspTraverserReadBuffer);

                auto start = (uint8*) &tempSuspTraverserReadBuffer[tagCE.offset];
                traverse_susp_tags(driver, start, start + tagCE.length, context, traverser);

                // No more data here. Moreover `tag` is not more valid (we previously called .get_device()->read(...)).
                break;
            }
            else if (mem_compare(tag->name, "ST", 2))
            {
                // This is just "terminate" field.
                break;
            }

            if (!traverser(context, tag))
                return;

            currentSuspTag += tag->length;
        }
    }
}
