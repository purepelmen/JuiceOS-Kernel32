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

#define BUS_CMD_CYLINDER_LOW(base) base + 4
#define BUS_CMD_CYLINDER_HIGH(base) base + 5
#define BUS_CMD_DRIVEHEAD(base) base + 6
#define BUS_CMD_STATUS(base) base + 7

#define BUS_CTRL_DEVCONTROL(base) base + CONTROL_PORT_OFFSET + 0

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

    static const char* ata_devtype_as_string(AtaDevType devType);
    // Selects the specified drive on the specified bus and waits for it to switch.
    static void ata_select_drive(uint16 busBase, bool isSlave);

    // Resets both the Master and Slave drive on the bus.
    static void ata_softreset(uint16 busBase);
    static AtaDevType ata_get_dev_type(uint16 busBase, bool isSlave);

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
            kconsole::printf("The primary master: %s.\n", ata_devtype_as_string(ata_get_dev_type(PRIMARY_CMD_IDE, false)));
            kconsole::printf("The primary slave: %s.\n", ata_devtype_as_string(ata_get_dev_type(PRIMARY_CMD_IDE, true)));
        }

        if (port_read8(BUS_CMD_STATUS(SECONDARY_CMD_IDE)) == 0xFF)
        {
            kernel_print_log("[IDE] Info: The secondary bus has no drives.\n");
        }
        else
        {
            kconsole::printf("The secondary master: %s.\n", ata_devtype_as_string(ata_get_dev_type(SECONDARY_CMD_IDE, false)));
            kconsole::printf("The secondary slave: %s.\n", ata_devtype_as_string(ata_get_dev_type(SECONDARY_CMD_IDE, true)));
        }
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

    void ata_select_drive(uint16 busBase, bool isSlave)
    {
        port_write8(BUS_CMD_DRIVEHEAD(busBase), 0xA | isSlave << 4);

        // Do a delay, so the active drive can switch.
        uint16 devControl = BUS_CTRL_DEVCONTROL(busBase);
        port_read8(devControl);
        port_read8(devControl);
        port_read8(devControl);
        port_read8(devControl);
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

        port_write8(BUS_CTRL_DEVCONTROL(busBase), 0);
    }

    AtaDevType ata_get_dev_type(uint16 busBase, bool isSlave)
    {
        ata_softreset(busBase);
        ata_select_drive(busBase, isSlave);

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
