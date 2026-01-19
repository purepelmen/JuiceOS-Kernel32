#pragma once
#include <stdint.h>

namespace kstorage
{
    class BlockDevice
    {
    public:
        virtual void read(uint32 start, uint8 count, uint16* outBuffer) = 0;
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

        void read(uint32 start, uint8 count, uint16* outBuffer) override
        {
            dev->read(offset + start, count, outBuffer);
        }

        uint32 get_total_sectors() override { return size; }
    };

    class FileSystem
    {
    protected:
        BlockDevice* device;
    
    public:
        void init(BlockDevice* device)
        {
            this->device = device;
            on_init();
        }

        virtual void read_dir() = 0;

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

    extern DriveInfo drives[MAX_DEVICES];
    extern int drivesCount;

    extern FileSystem* volumes[MAX_DEVICES];
    extern int volumeCount;

    void register_device(BlockDevice* dev);
    void init();

    void mount(DriveInfo* drive);
    void mount(BlockDevice* device);
}