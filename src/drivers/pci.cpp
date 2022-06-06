#include "drivers/screen.hpp"
#include "drivers/ports.hpp"
#include "drivers/pci.hpp"
#include "heap.hpp"

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

    void init()
    {
        devices = (pci_device*) malloc(sizeof(pci_device) * 100);
        int device_index = 0;

        for(int bus = 0; bus < 256; bus++)
        {
            for(int slot = 0; slot < 32; slot++)
            {
                for(int function = 0; function < 8; function++)
                {
                    pci_device device(bus, slot, function);

                    uint32 identifiers = device.read(0x0);
                    uint16 vendorid = identifiers & 0xFFFF;

                    if(vendorid == 0xFFFF || device.classid == 0xFF)
                    {
                        continue;
                    }

                    if(device_index > 99)
                    {
                        RAISE_ERROR("PCI found more than 100 devices (need array expansion)");
                    }

                    devices[device_index] = device;
                    device_index++;

                    print_dev_info(device);
                }
            }
        }
    }

    static void print_dev_info(pci_device& device)
    {
        kscreen::print_string("[PCI] Saving valid device (BSF=");
        print_number(device.bus);
        kscreen::print_string(",");
        print_number(device.slot);
        kscreen::print_string(",");
        print_number(device.function);
        
        kscreen::print_string(") with cl/subcl = ");
        print_number(device.classid);
        kscreen::print_string(",");
        print_number(device.subclass);
        kscreen::print_string("\n");
    }
}
