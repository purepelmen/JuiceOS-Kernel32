#include "storage.h"
#include "ide.h"

#include <heap.h>
#include <math.h>
#include <kernel.h>

#include <filesystems/iso_9660.h>
#include <filesystems/fat.h>
#include <filesystems/mbr.h>

using namespace kpart;

namespace kstorage
{
    static void analyze_partitioning(DriveInfo& drive);
    static FileSystem* probe_device(BlockDevice* device);

    static char tempReadBuffer[512];

    DriveInfo drives[MAX_DEVICES];
    int drivesCount = 0;

    FileSystem* volumes[MAX_DEVICES];
    int volumeCount = 0;

    void register_device(BlockDevice* dev)
    {
        if (drivesCount >= MAX_DEVICES)
            RAISE_ERROR_D("kstorage::register_device() failed", "Attempt to register another device, while the maxiumum of %d devices is reached.", MAX_DEVICES);
        
        DriveInfo& newDrive = drives[drivesCount++];
        newDrive.device = dev;
    }

    void register_partition(DriveInfo& driveInfo, Partition* partition)
    {
        if (driveInfo.childCount >= MAX_DEVICES)
            RAISE_ERROR_D("kstorage::register_partition() failed", "Attempt to register another parrition, while the maxiumum of %d is reached.", MAX_DEVICES);
        
        driveInfo.childs[driveInfo.childCount++] = partition;
    }

    void init()
    {
        // Analyze the partitioning
        kernel_log("[kstorage] Analyzing partitiong for %d registered drive(s)...\n", drivesCount);
        for (int i = 0; i < drivesCount; i++)
        {
            DriveInfo& drive = drives[i];
            analyze_partitioning(drive);
        }

        // Auto-mount everything.
        kernel_log("[kstorage] Trying to automount all found drives and partitions...\n", drivesCount);
        for (int i = 0; i < drivesCount; i++)
        {
            DriveInfo& drive = drives[i];
            mountDrive(&drive);
        }
    }

    void mountDrive(DriveInfo* drive)
    {
        mount(drive->device);
        
        for (int i = 0; i < drive->childCount; i++)
        {
            Partition* partDevice = drive->childs[i];
            mount(partDevice);
        }
    }

    void mount(BlockDevice* device)
    {
        if (volumeCount >= MAX_DEVICES)
            RAISE_ERROR_D("kstorage::mount() failed", "Attempt to mount another device, while the maxiumum of %d volumes is registered.", MAX_DEVICES);
        
        FileSystem* volume = probe_device(device);
        if (volume == nullptr)
            return;

        volume->init(device);
        volumes[volumeCount++] = volume;

        kernel_log("[kstorage] Mounted new device: blk dev size = %dKB.\n", device->get_total_sectors() * 512 / 1024);
    }

    void analyze_partitioning(DriveInfo& drive)
    {
        drive.device->read(0, 1, (uint16*)tempReadBuffer);

        mbr* mbr_s = (mbr*)tempReadBuffer;
        if(mbr_s->boot_sig != 0xAA55)
            return;

        for (int i = 0; i < 4; i++)
        {
            mbr_partition* part = &mbr_s->partitions[i];
            if (part->lba_size == 0x0)
                continue;

            uint32 startLba = part->start_lba;
            uint32 lbaSize = part->lba_size;
            register_partition(drive, kheap::create_new<Partition>(drive.device, startLba, lbaSize));

            kconsole::printf("Part #%d: Start=%d, Size=%d.\n", i, startLba, lbaSize);
        }
    }

    FileSystem* probe_device(BlockDevice* device)
    {
        FileSystem* selected;

        if (selected = kcd::probe(device))
            return selected;
        if (selected = kfat::probe(device))
            return selected;
        
        return nullptr;
    }

    size_t normalize_path(const char* path, char* outPath, size_t maxOutPathLen)
    {
        int outIdx = 0;
        int pathIdx = 0;

        while (path[pathIdx] != 0x0)
        {
            if (path[pathIdx] == '/' && (outIdx == 0 || outPath[outIdx - 1] != '/'))
            {
                outIdx += strlcpy("/", outPath + outIdx, maxOutPathLen - outIdx);
                pathIdx++;
            }

            for (; path[pathIdx] == '/'; pathIdx++);

            int i = pathIdx;
            for (; path[i] != '/' && path[i] != 0x0; i++);

            const char* segment = path + pathIdx;
            size_t segmentSize = i - pathIdx;
            pathIdx += segmentSize;

            if (segmentSize == 1 && *segment == '.')
                continue;

            if (segmentSize == 2 && mem_compare(segment, "..", 2))
            {
                if (outPath[outIdx] == '/')
                    outIdx--;
                while (outIdx > 0 && outPath[outIdx] != '/')
                    outIdx--;

                if (outPath[outIdx] == '/')
                    outIdx--;
                while (outIdx > 0 && outPath[outIdx] != '/')
                    outIdx--;

                outPath[outIdx] = 0x0;
                continue;
            }

            outIdx += strlcpy(segment, outPath + outIdx, min(segmentSize + 1, maxOutPathLen - outIdx));
        }

        return outIdx;
    }
}