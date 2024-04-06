#include "ports.hpp"
#include "pci.hpp"
#include "ide.hpp"

#include "../kernel.hpp"

#define CTRL_BASE_OFFSET 0x206

namespace kide
{
    const uint16 BUSES[] = { 0x1F0, 0x170 };

    static void ata_soft_reset(uint8 bus);

    void init()
    {
        kpci::pci_device* ide = kpci::find_device(0x01, 0x01);
        if(ide == nullptr)
        {
            kernel_print_log("[IDE] WARNING: Controller wasn't found in PCI device list\n");
            return;
        }
    }

    static void ata_soft_reset(uint8 bus)
    {
        // Do software reset
        port_byte_out(BUSES[bus] + CTRL_BASE_OFFSET, 4);
        port_byte_out(BUSES[bus] + CTRL_BASE_OFFSET, 0);

        // Wait 400ns
        for(int i = 0; i < 4; i++)
        {
            port_byte_in(BUSES[bus] + CTRL_BASE_OFFSET);
        }

        // Wait for device is ready and not busy
        uint8 status = 0;
        do
        {
            status = port_byte_in(BUSES[bus] + CTRL_BASE_OFFSET);
        }
        while(status & (1 << 6) || (status & (1 << 7)) == false);
    }
}
