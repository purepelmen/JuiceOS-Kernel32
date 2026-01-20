#pragma once
#include <drivers/storage.h>

namespace kcd
{
    enum iso9660_volumedesctype : uint8
    {
        BOOT_RECORD = 0,
        PRIMARY_DESC_VOLUME = 1,
        SUPPLEMENTARY_DESC_VOLUME = 2,
        VOLUME_PARTITION_DESC = 3,
        VOLUME_DESC_SET_TERMINATOR = 255,
    };

    struct iso9660_pvd_datetime
    {
        // Range 1-9999.
        char year[4];
        // Range 1-12.
        char month[2];
        // Range 1-31.
        char day[2];

        // Range 0-23.
        char hour[2];
        char minute[2];
        char second[2];
        // Range 0-99.
        char centiseconds[2];

        // Range 0-100. Described in 15 minute intervals. Where value 0 = GMT-12, and value 100 = GMT+13.
        uint8 timeZoneOffset;
    };

    struct iso9660_datetime
    {
        // Number of years since 1900.
        uint8 year;
        // Range 1-12.
        uint8 month;
        // Range 1-31.
        uint8 day;

        // Range 0-23.
        uint8 hour;
        // Range 0-59.
        uint8 minute;
        // Range 0-59.
        uint8 second;

        // Range 0-100. Described in 15 minute intervals. Where value 0 = GMT-12, and value 100 = GMT+13.
        uint8 timeZoneOffset;
    };

    struct iso9660_direntry
    {
        // Length of this dir record.
        uint8 length;
        uint8 extendedAttrRecordLength;

        uint32 locationLBA;
        uint32 locationLBA_BE;
        uint32 dataLength;
        uint32 dataLength_BE;

        iso9660_datetime recordingTime;
        uint8 flags;

        // Or zero.
        uint8 unitSizeOfFilesRecordedInInterleavedMode;
        // Or zero.
        uint8 interleaveGapSize;

        // Where the extent is recorded on.
        uint16 volumeSequenceNumber;
        uint16 volumeSequenceNumber_BE;

        uint8 filenameSize;

        // Has one byte of 0x0 padding in the end if the filename size is even.
        uint8 filenameStartByte;
    } __attribute__((packed));

    struct iso9660_volumedesc
    {
        iso9660_volumedesctype type;

        // Must be "CD001".
        char identifier[5];
        // Always 1.
        uint8 version;

        union
        {
            struct 
            {
                uint8 bootSystemId[32];
                uint8 bootId[32];

                uint8 customBootSystemSpecificData[1977];
            } __attribute__((packed)) bootrecord;

            struct 
            {
                uint8 unused;

                char systemId[32];
                char volumeId[32];

                uint8 unused2[8];

                uint32 volumeSpaceSize;
                uint32 volumeSpaceSize_BE;

                uint8 unused3[32];

                // Number of physicals disk this volume is subdivided on.
                uint16 volumeSetSize;
                uint16 volumeSetSize_BE;
                // The number of the disk in the volume set.
                uint16 volumeSequenceNumber;
                uint16 volumeSequenceNumber_BE;

                // The size in bytes of one logical block/sector (very commonly - 2048).
                uint16 logicalBlockSize;
                uint16 logicalBlockSize_BE;
                // The size in bytes of the path table (path table is a convenient index of all directories and subdirectories for fast traversal).
                uint32 pathTableSize;
                uint32 pathTableSize_BE;

                uint32 pathTableLBA_TypeL;
                uint32 pathTableLBA_TypeL_Optional;
                uint32 pathTableLBA_TypeM;
                uint32 pathTableLBA_TypeM_Optional;

                uint8 rootDirectoryEntry[34];

                // Volume set identifier this volume belongs to.
                uint8 volumeSetIdentifier[128];

                // Volume publisher (all spaces if not specified).
                uint8 publisherIdentifier[128];
                uint8 dataPreparerIdentifier[128];
                uint8 applicationIdentifier[128];
                
                // Filename of the file in the root dir that contains copyright information for this volume set (all spaces if not specified).
                uint8 copyrightFileIdentifier[37];
                // Filename of the file in the root dir that contains abstract information for this volume set (all spaces if not specified).
                uint8 abstractFileIdentifier[37];
                // Filename of the file in the root dir that contains bibliographic information for this volume set (all spaces if not specified).
                uint8 bibliographicFileIdentifier[37];

                iso9660_pvd_datetime volumeCreationTime;
                iso9660_pvd_datetime volumeModificationTime;
                iso9660_pvd_datetime volumeExpirationTime;
                iso9660_pvd_datetime volumeEffectiveTime;

                // Always 1.
                uint8 fileStructureVersion;
                uint8 unused4;

                // Contents not defined by ISO 9660.
                uint8 applicationData[512];

                // Reserved by ISO.
                uint8 reserved[653];
            } __attribute__((packed)) primary_volume_desc;

        } __attribute__((packed)) data;
    } __attribute__((packed));

    struct VolumeInfo
    {
        bool hasPVD;
        uint32 primaryVolumeDescLBA;

        uint8 rootDir[34];

        void setPrimaryVolumeDescLBA(uint32 primaryVolumeDescLBA)
        {
            hasPVD = true;
            primaryVolumeDescLBA = primaryVolumeDescLBA;
        }

        void setRootDirEntry(const iso9660_direntry* entry)
        {
            *((iso9660_direntry*)rootDir) = *entry;
        }

        const iso9660_direntry* getRootDirEntry()
        {
            return (iso9660_direntry*)rootDir;
        }
    };

    class ISO9660 : public kstorage::FileSystem
    {
    private:
        VolumeInfo volumeInfo;
    
    protected:
        void on_init() override;

    public:
        ISO9660(const VolumeInfo& info) : volumeInfo(info) {}
        void read_dir(const char* path, kstorage::ReadDirCallback callback) override;

    private:
        iso9660_direntry* resolve_path_part(uint32 parentLocationLBA, uint32 parentDataLength, const char* part);
    };

    kstorage::FileSystem* probe(kstorage::BlockDevice* device);
}