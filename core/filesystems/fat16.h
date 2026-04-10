#pragma once
#include <drivers/storage.h>

#define FATTR_READONLY     0x01
#define FATTR_HIDDEN       0x02
#define FATTR_SYSTEM       0x04
#define FATTR_VOLUMELABEL  0x08
#define FATTR_SUBDIRECTORY 0x10
#define FATTR_ARCHIVE      0x20
#define FATTR_DEVICE       0x40
#define FATTR_UNUSED       0x80

#define FATTR_LFN          0x0F

namespace kfat
{
    struct fat_bpb
    {
        uint8 jmp_code[3];
        uint8 oem_id[8];

        uint16 bytes_per_sector;
        uint8 sectors_per_cluster;
        uint16 reserved_sectors;
        uint8 fat_count;
        uint16 root_dir_entries;
        uint16 total_sectors;
        uint8 media_desc_type;
        uint16 sectors_per_fat;
        uint16 sectors_per_track;
        uint16 heads_count;
        uint32 hidden_sectors_count;
        uint32 large_sector_count;

        uint8 drive_num;
        uint8 winnt_flags;
        uint8 signature;
        uint32 volume_id;
        uint8 volume_label[11];
        uint8 system_id_str[8];

        uint8 boot_code[448];
        uint16 boot_sig;
    } __attribute__((packed));

    struct dir_entry
    {
        uint8 dos_filename[8];
        uint8 dos_fileext[3];
        uint8 attributes;
        uint8 reserved;
        uint8 creation_time_fineres;
        uint16 creation_time;
        uint16 creation_date;
        uint16 last_access_date;
        uint16 not_used;
        uint16 last_modified_time;
        uint16 last_modified_date;
        uint16 first_cluster;
        uint32 file_size;
    } __attribute__((packed));

    struct dir_lfn_entry
    {
        uint8 seq_num;
        uint16 name_chars[5];
        uint8 attributes;
        uint8 reserved;
        uint8 checksum;
        uint16 name_chars2[6];
        uint16 zero_reserved;
        uint16 name_chars3[2];
    } __attribute__((packed));

    enum class FatType
    {
        FAT12,
        FAT16,
        FAT32,
        ExFAT
    };

    struct VolumeInfo
    {
        FatType type;
        char bpbVolumeLabel[12];
        
        uint16 bytesPerSector;
        uint8 sectorsPerCluster;
        uint16 reservedSectors;
        uint16 sectorsPerFAT;
        uint16 rootDirEntries;
        uint8 fatCount;

        uint32 totalSectors;

        uint32 fatLBA;
        uint32 rootDirectoryLBA;
        uint32 dataLBA;

        uint32 rootDirSectors;
        uint32 dataSectors;
        uint32 totalClusters;

        kstorage::FileState rootDirectoryFile;
    };

    class FAT : public kstorage::FileSystem
    {
    private:
        VolumeInfo volumeInfo;
        uint32 chainEndCluster;
        char volumeName[32];

        uint8* loadedFAT;
        size_t loadedFATPageCount;
    
        uint8* buffer = 0;
    
    protected:
        void on_init() override;

    public:
        FAT(const VolumeInfo& info) : volumeInfo(info) {}

        const char* get_name() override;
        size_t get_size() override;

        bool resolve_path(const char* path, kstorage::FileState& state) override;
        size_t read(kstorage::FileState& state, char* buffer, size_t length) override;
        void read_dir(const char* path, kstorage::ReadDirCallback callback, void* context) override;

    private:
        bool find_volumelabel(char* outLabel, size_t maxLabelSize);
        bool resolve_path_part(uint32 cluster, const char* part, kstorage::FileState& outResolved);
        uint32 next_cluster(uint32 cluster);

        uint8* read(uint32 lba);
    };

    kstorage::FileSystem* probe(kstorage::BlockDevice* device);
}
