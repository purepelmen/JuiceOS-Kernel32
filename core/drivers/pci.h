#pragma once
#include "../stdlib.h"

namespace kpci
{
    class pci_device
    {
public:
        pci_device(uint8 bus, uint8 slot, uint8 function);
        pci_device();

        void write(uint16 offset, uint32 value);
        uint32 read(uint16 offset);

        bool is_present();
        
        uint8 bus = 0;
        uint8 slot = 0;
        uint8 function = 0;

        uint32 address = 0;
        uint8 classid = 0;
        uint8 subclass = 0; 
    };

    extern pci_device* devices;

    void init();
    pci_device* find_device(uint8 classid, uint8 subclass);
}
