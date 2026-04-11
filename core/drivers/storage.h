#pragma once
#include <stdint.h>

namespace kstorage
{
    class BlockDevice
    {
    public:
        /// @brief Read the specified `count` of sectors, starting from `start` sector index.
        /// @param outBuffer Where to put, currently with no alignment requirements.
        virtual void read(uint32 start, uint32 count, uint16* outBuffer) = 0;

        /// @brief Returns the amount of sectors (always 512-byte sized).
        virtual uint32 get_total_sectors() = 0;
    };

    class Partition : public BlockDevice
    {
    private:
        BlockDevice* dev;
        uint32 offset;
        uint32 size;

    public:
        Partition(BlockDevice* dev, uint32 offset, uint32 size) : dev(dev), offset(offset), size(size) {}

        void read(uint32 start, uint32 count, uint16* outBuffer) override
        {
            dev->read(offset + start, count, outBuffer);
        }

        uint32 get_total_sectors() override { return size; }
        BlockDevice* get_base() const { return dev; }
    };

    struct FileState
    {
        uint32 inode;
        uint32 size;
        uint8 flags;

        size_t position = 0;

        size_t is_eof() const { return size - position <= 0; }
    };

    const size_t MAX_FILENAME_SIZE = 256;

    enum class DirEntryType
    {
        DIRECTORY,
        FILE,
        SPECIAL
    };

    struct DirEntry
    {
        DirEntryType type;
        
        char name[MAX_FILENAME_SIZE];
        FileState fileState;
    };

    typedef bool (*ReadDirCallback)(void* context, DirEntry& entry);

    class FileSystem
    {
    protected:
        BlockDevice* device = nullptr;
    
    public:
        void init(BlockDevice* device)
        {
            this->device = device;
            on_init();
        }

        BlockDevice* get_device() const { return device; }
        
        virtual const char* get_name() = 0;
        virtual const char* get_label() = 0;
        virtual size_t get_size() = 0;
        virtual size_t get_usable_size() { return get_size(); }

        virtual bool resolve_path(const char* path, FileState& state) = 0;
        virtual size_t read_file(FileState& state, char* buffer, size_t length) = 0;
        virtual void read_dir(const char* path, ReadDirCallback callback, void* context) = 0;

    protected:
        virtual void on_init() = 0;
    };

    const int MAX_DEVICES = 16;

    struct DriveInfo
    {
        BlockDevice* device;

        Partition* childs[MAX_DEVICES];
        int childCount = 0;
    };

    /// @brief Registered hardware drives by disk drivers.
    extern DriveInfo drives[MAX_DEVICES];
    extern int drivesCount;

    /// @brief List of mounted volumes.
    extern FileSystem* volumes[MAX_DEVICES];
    extern int volumeCount;

    void register_device(BlockDevice* dev);
    void init();

    void mount_drive(DriveInfo* drive);
    bool mount(BlockDevice* device);

    size_t normalize_path(const char* path, char* outPath, size_t maxOutPathLen);
}