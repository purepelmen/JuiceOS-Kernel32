#pragma once
#include <stdint.h>

namespace kide
{
    enum class AtaDevType
    {
        UNKNOWN,
        PATA,
        SATA,
        PATAPI,
        SATAPI
    };

    struct AtaDevice
    {
        AtaDevType type;

        uint32 bus;
        bool isSlave;

        char name[20];
        char model[41];

        uint32 totalAddressableSectors;
    };

    const int MAX_DEVICES = 4;

    extern AtaDevice devices[MAX_DEVICES];
    extern int deviceCount;

    void init();
    bool ata_read_sector(uint16 busBase, bool isSlave, uint32 startLba, uint8 sectorCount, uint16* buffer);

    const char* ata_devtype_as_string(AtaDevType devType);
}
