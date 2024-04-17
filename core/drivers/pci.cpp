#include "screen.h"
#include "ports.h"
#include "pci.h"

#include <kernel.h>
#include <heap.h>
#include <console.h>

namespace kpci
{
    pci_device* devices;

    static void print_dev_info(pci_device& device);

    pci_device::pci_device(uint8 bus, uint8 slot, uint8 function) : bus(bus), slot(slot), function(function)
    {
        address = 0x80000000 | (bus << 16) | (slot << 11) | (function << 8);

        uint32 classcodes = read(0x8);
        classid = (classcodes >> 24) & 0xFF;
        subclass = (classcodes >> 16) & 0xFF;
    }

    pci_device::pci_device() : bus(0), slot(0), function(0)
    {}

    void pci_device::write(uint16 offset, uint32 value)
    {
        uint32 n_address = address | offset;
        port_dword_out(0xCF8, n_address);

        port_dword_out(0xCFC, value);
    }

    uint32 pci_device::read(uint16 offset)
    {
        uint32 n_address = address | offset;
        port_dword_out(0xCF8, n_address);

        return port_dword_in(0xCFC);
    }

    bool pci_device::is_present()
    {
        uint32 identifiers = read(0x0);
        uint16 vendor_id = identifiers & 0xFFFF;

        return vendor_id != 0xFFFF && classid != 0xFF;
    }

    void init()
    {
        const int MAX_DEVICES = 100;

        devices = kheap::create_new_array<pci_device>(MAX_DEVICES);
        // TODO: Probably it's not needed anymore. Delete it later.
        // mem_fill((uint8*) devices, 0, sizeof(pci_device) * MAX_DEVICES);

        int device_index = 0;
        for(int bus = 0; bus < 256; bus++)
        {
            for(int slot = 0; slot < 32; slot++)
            {
                for(int function = 0; function < 8; function++)
                {
                    pci_device device(bus, slot, function);
                    if(device.is_present() == false) continue;

                    if(device_index > (MAX_DEVICES - 1))
                    {
                        RAISE_ERROR("PCI found more devices than allowed (need array expansion)");
                    }

                    devices[device_index] = device;
                    device_index++;

                    print_dev_info(device);
                }
            }
        }

        kernel_print_log("PCI driver init completed.\n");
    }

    pci_device* find_device(uint8 classid, uint8 subclass)
    {
        for(int i = 0; i < 100 && devices[i].address != 0; i++)
        {
            if(devices[i].classid == classid && devices[i].subclass == subclass)
            {
                return &devices[i];
            }
        }

        return nullptr;
    }

    static void print_dev_info(pci_device& device)
    {
        kconsole::printf("[PCI] Saving valid device (BSF=%d,%d,%d) with cl/subcl = %d,%d\n", device.bus, device.slot, device.function, 
            device.classid, device.subclass);
        
        // kscreen::print_string("[PCI] Saving valid device (BSF=");
        // kconsole::print_decimal(device.bus);
        // kscreen::print_string(",");
        // kconsole::print_decimal(device.slot);
        // kscreen::print_string(",");
        // kconsole::print_decimal(device.function);
        
        // kscreen::print_string(") with cl/subcl = ");
        // kconsole::print_decimal(device.classid);
        // kscreen::print_string(",");
        // kconsole::print_decimal(device.subclass);
        // kscreen::print_string("\n");
    }
}
