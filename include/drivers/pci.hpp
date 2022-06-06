#pragma once
#include "stdlib.hpp"

namespace kpci
{
    class pci_device
    {
public:
        pci_device(uint8 bus, uint8 slot, uint8 function);
        pci_device();

        void write(uint16 offset, uint32 value);
        uint32 read(uint16 offset);
        
        uint8 bus;
        uint8 slot;
        uint8 function;

        uint32 address;
        uint8 classid;
        uint8 subclass; 
    };

    extern pci_device* devices;

    void init();
}
