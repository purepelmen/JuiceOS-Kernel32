#include "ports.h"
#include "pci.h"
#include "ide.h"

#include <kernel.h>
#include <console.h>

#define CONTROL_PORT_OFFSET 0x206

#define PRIMARY_CMD_IDE 0x1F0
#define SECONDARY_CMD_IDE 0x170
#define PRIMARY_CONTRL_IDE PRIMARY_CMD_IDE + CONTROL_PORT_OFFSET
#define SECONDARY_CONTRL_IDE SECONDARY_CMD_IDE + CONTROL_PORT_OFFSET

#define BUS_CMD_DATA(base) base + 0
#define BUS_CMD_SECTOR_COUNT(base) base + 2
#define BUS_CMD_SECTOR_NUMBER(base) base + 3
#define BUS_CMD_CYLINDER_LOW(base) base + 4
#define BUS_CMD_CYLINDER_HIGH(base) base + 5
#define BUS_CMD_DRIVEHEAD(base) base + 6
#define BUS_CMD_STATUS(base) base + 7
#define BUS_CMD_COMMAND(base) base + 7

#define BUS_CTRL_DEVCONTROL(base) base + CONTROL_PORT_OFFSET + 0

namespace kide
{
    AtaDevice devices[4];
    int deviceCount = 0;

    static void ata_register_device(const char* name, uint16 busBase, bool isSlave);

    static bool ata_identify(AtaDevice* outDevice);
    static void ata_analyze_identify(AtaDevice* device, uint16* identifyData);
    static bool ata_pio_read_poll(uint16 busBase, uint16* buffer, uint8 sectorCount = 1);

    // Selects the specified drive on the specified bus and waits for it to switch. Optionally sets the head number or the highest last 4 bits of LBA.
    static void ata_select_drive(uint16 busBase, bool isSlave, uint8 headOrLBA24_27_bits = 0);

    // Resets both the Master and Slave drive on the bus.
    static void ata_softreset(uint16 busBase);
    static AtaDevType ata_read_dev_type(uint16 busBase);

    void init()
    {
        kpci::pci_device* ide = kpci::find_device(0x01, 0x01);
        if(ide == nullptr)
        {
            kernel_print_log("[IDE] WARNING: Controller wasn't found in PCI device list.\n");
            return;
        }

        // Check for floating buses. The status is 0xFF if no there are no drives (not definite though).
        if (port_read8(BUS_CMD_STATUS(PRIMARY_CMD_IDE)) == 0xFF)
        {
            kernel_print_log("[IDE] Info: The primary bus has no drives.\n");
        }
        else
        {
            ata_register_device("PRIMARY MASTER", PRIMARY_CMD_IDE, false);
            ata_register_device("PRIMARY SLAVE", PRIMARY_CMD_IDE, true);
        }

        // Check the secondary bus for floating.
        if (port_read8(BUS_CMD_STATUS(SECONDARY_CMD_IDE)) == 0xFF)
        {
            kernel_print_log("[IDE] Info: The secondary bus has no drives.\n");
        }
        else
        {
            ata_register_device("SECONDARY MASTER", SECONDARY_CMD_IDE, false);
            ata_register_device("SECONDARY SLAVE", SECONDARY_CMD_IDE, true);
        }
    }

    void ata_register_device(const char* name, uint16 busBase, bool isSlave)
    {
        if (deviceCount >= MAX_DEVICES)
        {
            kernel_print_log("[IDE] Error: ata_register_device failed to register another device. The maximum is reached.\n");
            return;
        }

        AtaDevice newDevice;
        mem_fill(&newDevice, 0, sizeof(AtaDevice));

        newDevice.bus = busBase;
        newDevice.isSlave = isSlave;
        strcpy(name, newDevice.name);
        
        bool isIdentified = ata_identify(&newDevice);
        if (isIdentified)
        {
            devices[deviceCount] = newDevice;
            deviceCount++;
        }
    }

    bool ata_identify(AtaDevice* device) 
    {
        uint32 busBase = device->bus; bool isSlave = device->isSlave;
        ata_select_drive(busBase, isSlave);

        port_write8(BUS_CMD_SECTOR_COUNT(busBase), 0);
        port_write8(BUS_CMD_SECTOR_NUMBER(busBase), 0);
        port_write8(BUS_CMD_CYLINDER_LOW(busBase), 0);
        port_write8(BUS_CMD_CYLINDER_HIGH(busBase), 0);

        // Finally send the IDENTIFY command.
        port_write8(BUS_CMD_COMMAND(busBase), 0xEC);

        if (port_read8(BUS_CMD_STATUS(busBase)) == 0)
        {
            kconsole::printf("[IDE] ATA Device (BUS=%x SLAVE=%d): drive is not connected.\n", busBase, isSlave);
            return false;
        }

        // Wait until BSY is cleared.
        while (port_read8(BUS_CMD_STATUS(busBase)) & 1 << 7);

        device->type = ata_read_dev_type(busBase);
        kconsole::printf("[IDE] ATA Device (BUS=%x SLAVE=%d) is detected as %s.\n", busBase, isSlave, ata_devtype_as_string(device->type));
        
        if (device->type != AtaDevType::PATA && device->type != AtaDevType::SATA)
        {
            kconsole::printf("Skipping non ATA or SATA device. Packet devices aren't supported yet.\n");
            return false;
        }

        uint16 identifyData[256];
        bool readSuccess = ata_pio_read_poll(busBase, identifyData, 1);

        if (!readSuccess)
        {
            kconsole::printf("IDENTIFY failed: pio read operation has not completed with success.\n");
            return false;
        }
        kconsole::printf("IDENTIFY completed.\n");
        
        ata_analyze_identify(device, identifyData);
        kconsole::printf("IDENTIFY Model: %s\n", device->model);
        kconsole::printf("IDENTIFY: LBA48 supported=%d, Total addressable space=%dKB\n", (identifyData[83] & 1 << 10) != 0, device->totalAddressableSectors / 2);

        return true;
    }

    void ata_analyze_identify(AtaDevice* device, uint16* identifyData)
    {
        // Extract model string (word 27–46, 40 bytes).
        char model[41];
        for (int i = 0; i < 40; i += 2)
        {
            model[i] = (char)(identifyData[27 + i/2] >> 8);
            model[i + 1] = (char)(identifyData[27 + i/2] & 0xFF);
        }
        model[40] = 0;

        // Trim at the end to remove unnecessary spaces.
        int i = 39;
        while (model[i] == 0x20)
        {
            model[i] = 0x0;
            i--;
        }
        mem_copy(model, device->model, 41);

        device->totalAddressableSectors = *((int*)&identifyData[60]);
    }

    bool ata_read_sector(uint16 busBase, bool isSlave, uint32 startLba, uint8 sectorCount, uint16* buffer) 
    {
        ata_select_drive(busBase, isSlave, startLba >> 24);

        // Wait until BSY is clear.
        while (port_read8(BUS_CMD_STATUS(busBase)) & (1 << 7));

        port_write8(BUS_CMD_SECTOR_COUNT(busBase), sectorCount);
        port_write8(BUS_CMD_SECTOR_NUMBER(busBase), startLba & 0xFF);
        port_write8(BUS_CMD_CYLINDER_LOW(busBase), (startLba >> 8) & 0xFF);
        port_write8(BUS_CMD_CYLINDER_HIGH(busBase), (startLba >> 16) & 0xFF);

        // Issue READ SECTORS command.
        port_write8(BUS_CMD_COMMAND(busBase), 0x20);
        
        return ata_pio_read_poll(busBase, buffer, sectorCount);
    }

    bool ata_pio_read_poll(uint16 busBase, uint16* buffer, uint8 sectorCount)
    {
        for (int sectorIdx = 0; sectorIdx < sectorCount; sectorIdx++)
        {
            uint8 status;

            // Wait until DRQ is set meaning it has PIO data to transfer. Additionally checking if an error occured.
            do 
            {
                status = port_read8(BUS_CMD_STATUS(busBase));
                if (status & 1)
                {
                    kconsole::printf("[IDE] ERR flag set during reading sector number %d.\n", sectorIdx);
                    return false;
                }
            } 
            while ((status & 1 << 3) == 0);

            // Read next 256 words to the buffer.
            for (int j = 0; j < 256; j++)
            {
                buffer[sectorIdx * 256 + j] = port_read16(BUS_CMD_DATA(busBase));
            }
            
            // 400ns delay to ensure DRQ is cleared, possibly BSY may be set.
            for (int i = 0; i < 4; i++)
            {
                port_read8(BUS_CTRL_DEVCONTROL(busBase));
            }
        }

        return true;
    }

    const char* ata_devtype_as_string(AtaDevType devType)
    {
        switch (devType)
        {
            case AtaDevType::UNKNOWN: return "UNKNOWN DEVICE";
            case AtaDevType::PATA: return "PATA";
            case AtaDevType::SATA: return "SATA";
            case AtaDevType::PATAPI: return "PATAPI";
            case AtaDevType::SATAPI: return "SATAPI";

            default: return "__UNKNOWN_ENUM_TYPE";
        }
    }

    void ata_select_drive(uint16 busBase, bool isSlave, uint8 headOrLBA24_27_bits)
    {
        port_write8(BUS_CMD_DRIVEHEAD(busBase), 0xE0 | isSlave << 4 | (headOrLBA24_27_bits & 0x0F));

        // Wait until the device is switched.
        for (int i = 0; i < 4; i++)
        {
            port_read8(BUS_CTRL_DEVCONTROL(busBase));
        }
    }

    void ata_softreset(uint16 busBase)
    {
        // NOTE: Does this may reset something important we needn't change?
        port_write8(BUS_CTRL_DEVCONTROL(busBase), 1 << 2);

        // Delay must be at least 5 microseconds (the code may execute a little longer).
        for (int i = 0; i < 256; i++)
        {
            port_read8(BUS_CTRL_DEVCONTROL(busBase));
        } 

        // Must set it back to zero.
        port_write8(BUS_CTRL_DEVCONTROL(busBase), 0);
    }

    AtaDevType ata_read_dev_type(uint16 busBase)
    {
        uint8 cl = port_read8(BUS_CMD_CYLINDER_LOW(busBase));
	    uint8 ch = port_read8(BUS_CMD_CYLINDER_HIGH(busBase));

        if (cl==0x14 && ch==0xEB)
            return AtaDevType::PATAPI;
        if (cl==0x69 && ch==0x96)
            return AtaDevType::SATAPI;
        if (cl==0 && ch == 0)
            return AtaDevType::PATA;
        if (cl==0x3c && ch==0xc3)
            return AtaDevType::SATA;

        return AtaDevType::UNKNOWN;
    }
}
