#include "shell.h"
#include "heap.h"
#include "console.h"
#include "kernel.h"

#include "drivers/screen.h"
// #include "drivers/ahci.h"
#include "drivers/pit.h"
#include "drivers/ps2.h"
#include "drivers/pci.h"
#include "drivers/ide.h"

#include "filesystems/fat16.h"

kfat::FAT16* fat_pt2;

static void open_memdumper(void);
static void open_syslogs(void);
static void open_debugger(void);

void print_systemcpu(void);

void kshell::open_console(void)
{
    kscreen::clear();

    while(true)
    {
        kscreen::outargs.print_color = 0x02;
        kscreen::print_string("PC:>>");
        kscreen::outargs.print_color = SCREEN_STDCOLOR;

        string command = kconsole::read_string();
        command.to_lower(command);

        if(command.length() == 0)
        {
            // If nothing was printed
            continue;
        }

        if(command == "hello")
        {
            kscreen::print_string("Helloooo :)\n\n");
            continue;
        }

        // Awaiting IDT re-implementation
        // TO FIX: This not working (all just hangs) 
        if(command == "reboot")
        {
            kscreen::print_string("This command doesn't work for now. It will be fixed soon.\n\n");
            continue;
        }

        if(command == "cls")
        {
            kscreen::clear();
            continue;
        }

        if(command == "system")
        {
            kscreen::print_string("JuiceOS Kernel32 v" KERNEL_VERSION "\n");
            print_systemcpu();
            continue;
        }

        if(command == "systemcpu")
        {
        	print_systemcpu();
        	continue;
        }

        if(command == "exit")
        {
            return;
        }

        if(command == "ascii")
        {
            kscreen::print_string("Type any char.\n");

            uint8 key = kps2::read_ascii();
            kscreen::print_string("ASCII code of typed key is: 0x");
            kconsole::print_hex(key, 2);
            kscreen::print_string("\nIt displays as: ");

            kscreen::print_char(key);
            kscreen::print_string("\n\n");
            continue;
        }

        if(command == "memdump")
        {
            open_memdumper();
            kscreen::clear();
            continue;
        }

        if(command == "help")
        {
            kscreen::print_string("ASCII - Print hex representation of a typed char.\n");
            kscreen::print_string("CLS - Clear the console.\n");
            kscreen::print_string("EXIT - Quit from console to OS menu.\n");
            kscreen::print_string("HELP - Print this message.\n");
            kscreen::print_string("HELLO - Test command that say hello to you.\n");
            kscreen::print_string("MEMDUMP - Open Memory dumper.\n");
            kscreen::print_string("REBOOT - Reboot your PC.\n");
            kscreen::print_string("SCANTEST - Print scancode of every pressed key.\n");
            kscreen::print_string("SYSTEM - Print system information.\n");
            kscreen::print_string("SYSTEMCPU - Print CPU information.\n");
            kscreen::print_string("PCI - Print all PCI devices.\n");
            // kscreen::print_string("AHCIVER - Print AHCI specification version.\n");
            // kscreen::print_string("AHCIDEV - Print all AHCI ports and connected devices.\n");
            // kscreen::print_string("AHCIRD - Read first sector from AHCI port #0.\n");
            kscreen::print_string("IDEDEV - Print all ATA devices from IDE driver.\n");
            kscreen::print_string("IDERD - Read first sector from the first IDE device.\n");

            kscreen::print_char('\n');
            continue;
        }

        if(command == "scantest")
        {
            kps2::read_scancode(false);

            while(true)
            {
                uint8 scancode = kps2::read_scancode(false);

                kscreen::print_string("0x");
                kconsole::print_hex(scancode, 2);

                kscreen::print_char('\n');
                if(scancode == 0x81) break;
            }
            
            kscreen::print_char('\n');
            continue;
        }

        if(command == "pci")
        {
            for(int i = 0; i < 100 && kpci::devices[i].address != 0; i++)
            {
                kpci::pci_device* device = &kpci::devices[i];
                kconsole::printf("PCI Device (Bus/Slot/Fun: %d,%d,%d) = Class/Subclass: %d,%d\n", 
                    device->bus, device->slot, device->function, device->classid, device->subclass);
            }

            kscreen::print_char('\n');
            continue;
        }

        // if(command == "ahciver")
        // {
        //     kconsole::printf("[AHCI] Version of the specification: %s\n", kahci::get_version());
        //     kscreen::print_char('\n');
        //     continue;
        // }

        // if(command == "ahcidev")
        // {
        //     if(kahci::hba_memory != nullptr)
        //     {
        //         uint32 imp_ports = kahci::hba_memory->pi;
        //         string dev_names[] = { "Device not present", "SATAPI", "SEMB", "Port multiplier", "SATA" };

        //         for(int i = 0; i < 32; i++)
        //         {
        //             if(imp_ports & 1)
        //             {
        //                 kahci::port_devtype dev_status = kahci::get_port_devtype(&kahci::hba_memory->ports[i]);
        //                 kconsole::printf("[AHCI] Port %d device type: %s\n", i, dev_names[dev_status]);
        //             }

        //             imp_ports >>= 1;
        //         }
        //     }
        //     else
        //     {
        //         kscreen::print_string("AHCI driver wasn't initialized\n");
        //     }

        //     kscreen::print_char('\n');
        //     continue;
        // }

        // if(command == "ahcird")
        // {
        //     uint8 buff[1024];
        //     bool result = kahci::read(0, 0, 0, 1, (uint16*) &buff);

        //     if(result)
        //     {
        //         for(int i = 0; i < 256; i++)
        //         {
        //             kconsole::print_hex8(buff[i]);
        //             kscreen::print_char(' ');
        //         }

        //         kscreen::print_char('\n');
        //     }
        //     else
        //     {
        //         kconsole::printf("Failed to read data\n");
        //     }

        //     kscreen::print_char('\n');
        //     continue;
        // }

        if (command == "idedev")
        {
            for (int i = 0; i < 4; i++)
            {
                kide::AtaDevice device = kide::devices[i];
                if (device.type == kide::AtaDevType::UNKNOWN)
                    continue;

                kconsole::printf("[IDE] Device %d: %s ('%s'); %s\n", i, device.name, device.model, kide::ata_devtype_as_string(device.type));
                kconsole::printf("      Addressable space: %dKB (%d sectors).\n", device.totalAddressableSectors / 2, device.totalAddressableSectors);
            }

            kscreen::print_char('\n');
            continue;
        }

        if (command == "iderd")
        {
            kconsole::printf("ATA device number: ");
            
            unsigned deviceNum = str_to_uint(kconsole::read_string().ptr());
            if (deviceNum >= kide::deviceCount)
            {
                kconsole::printf("No ATA device found, there's only %d of them.\n\n", kide::deviceCount);
                continue;
            }

            kide::AtaDevice* device = &kide::devices[deviceNum];

            uint8 buff[512];
            if (kide::ata_read_sector(device->bus, device->isSlave, 0, 1, (uint16*)buff))
            {
                for(int i = 0; i < 256; i++)
                {
                    kconsole::print_hex(buff[i], 2);
                    kscreen::print_char(' ');
                }

                kscreen::print_char('\n');
            }
            else
            {
                kconsole::printf("Failed to read test data from an ATA device.\n");
            }

            kscreen::print_char('\n');
            continue;
        }

        if(command == "fatinit")
        {
            kconsole::printf("Initializing FAT on partition #2\n");
            fat_pt2 = kheap::create_new<kfat::FAT16>(0);
            fat_pt2->init(1);

            kconsole::printf("\n");
            continue;
        }

        if(command == "fatdir")
        {
            kconsole::printf("K32 Partition files\n\n");
            fat_pt2->dir(0);

            kconsole::printf("\n");
            continue;
        }

        if(command == "\x1B")
            return;
        
        kscreen::print_string("Unknown command.\n\n");
    }
}

void kshell::open_menu(void)
{
    kscreen::clear();
    const uint8 NON_SELECTED_COLOR = SCREEN_STDCOLOR;
    const uint8 SELECTED_COLOR = SCREEN_INVERTCOLOR;
    const uint8 ITEMS_AMOUNT = 3;         // Zero means - 1

    uint8 currentPosition = 0;
    while(true)
    {
        kscreen::outargs.print_color = NON_SELECTED_COLOR;

        kscreen::outargs.cursor_x = 25;
        kscreen::outargs.cursor_y = 1;
        kscreen::print_string("Juice OS v" KERNEL_VERSION " Menu");

        kscreen::outargs.cursor_x = 6;
        kscreen::outargs.cursor_y = 3;
        if(currentPosition == 0)
            kscreen::outargs.print_color = SELECTED_COLOR;
        else 
            kscreen::outargs.print_color = NON_SELECTED_COLOR;
        kscreen::print_string("Open console");

        kscreen::outargs.cursor_x = 6;
        kscreen::outargs.cursor_y = 4;
        if(currentPosition == 1)
            kscreen::outargs.print_color = SELECTED_COLOR;
        else 
            kscreen::outargs.print_color = NON_SELECTED_COLOR;
        kscreen::print_string("Memory dumper");

        kscreen::outargs.cursor_x = 6;
        kscreen::outargs.cursor_y = 5;
        if(currentPosition == 2)
            kscreen::outargs.print_color = SELECTED_COLOR;
        else 
            kscreen::outargs.print_color = NON_SELECTED_COLOR;
        kscreen::print_string("System logs");

        kscreen::outargs.cursor_x = 6;
        kscreen::outargs.cursor_y = 6;
        if(currentPosition == 3)
            kscreen::outargs.print_color = SELECTED_COLOR;
        else 
            kscreen::outargs.print_color = NON_SELECTED_COLOR;
        kscreen::print_string("Debug");

        // Getting input
        uint8 key = kps2::read_scancode(true);
        if(key == 0x48)
        {
            if(currentPosition == 0) currentPosition = ITEMS_AMOUNT;
            else currentPosition--;
        }
        if(key == 0x50)
        {
            if(currentPosition == ITEMS_AMOUNT) currentPosition = 0;
            else currentPosition++;
        }

        if(key == 0x1C)
        {
            kscreen::outargs.print_color = NON_SELECTED_COLOR;
            if(currentPosition == 0)
            {
                kshell::open_console();
                kscreen::clear();
            }
            if(currentPosition == 1)
            {
                open_memdumper();
                kscreen::clear();
            }
            if(currentPosition == 2)
            {
                open_syslogs();
                kscreen::clear();
            }
            if(currentPosition == 3)
            {
                open_debugger();
                kscreen::clear();
            }
        }
    }
}

void open_memdumper(void)
{
    uint8* memPtr = (uint8*) 0x0;
    uint8 asciiFlag = false;
    kscreen::clear();

    while(true)
    {
        kscreen::outargs.print_color = SCREEN_INVERTCOLOR;
        kscreen::print_string_raw("                             Juice OS Memory Dumper                             ");

        kscreen::outargs.print_color = SCREEN_STDCOLOR;
        kscreen::outargs.cursor_x = 0;
        kscreen::outargs.cursor_y = 2;

        kscreen::print_string_raw("    Offset: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");

        kscreen::outargs.cursor_x = 0;
        kscreen::outargs.cursor_y = 4;

        for(int i = 0; i < 256; i++)
        {
            if(asciiFlag)
            {
                kscreen::print_string_raw("0x");
                kconsole::print_hex((uint32) memPtr + i, 8);
                kscreen::print_string_raw(": ");

                for(int ii = 0; ii < 16; ii++)
                {
                    kscreen::outargs.print_color = 0x07;
                    kscreen::print_char_raw(memPtr[i + ii]);
                    kscreen::outargs.print_color = SCREEN_STDCOLOR;
                    kscreen::print_string_raw("  ");
                }

                i += 15;
                kscreen::print_char('\n', false);
            } 
            else
            {
                kscreen::print_string_raw("0x");
                kconsole::print_hex((uint32) memPtr + i, 8);
                kscreen::print_string_raw(": ");

                for(int ii = 0; ii < 16; ii++)
                {
                    kscreen::outargs.print_color = 0x07;
                    kconsole::print_hex(memPtr[i + ii], 2);
                    kscreen::outargs.print_color = SCREEN_STDCOLOR;
                    kscreen::print_char_raw(' ');
                }

                i += 15;
                kscreen::print_char('\n', false);
            }
        }

        kscreen::outargs.print_color = SCREEN_INVERTCOLOR;
        kscreen::outargs.cursor_x = 0;
        kscreen::outargs.cursor_y = 24;
        kscreen::print_string_raw("Dump: 0x");
        kconsole::print_hex((uint32) memPtr, 8);
        kscreen::print_string_raw(" - 0x");
        kconsole::print_hex((uint32) memPtr + 255, 8);
        kscreen::print_string_raw(" | ASCII Flag = ");
        if(asciiFlag)
            kscreen::print_string_raw("ON ");
        else
            kscreen::print_string_raw("OFF");
        kscreen::print_string_raw("                                     ");

        kscreen::outargs.cursor_x = 0;
        kscreen::outargs.cursor_y = 0;
        kscreen::update_cursor();

        uint8 key = kps2::read_scancode(true);
        if(key == 0x01)
            break;
        if(key == 0x4D)
            // Right arrow
            memPtr += 0x100;
        if(key == 0x4B && (int) memPtr - 0x100 >= 0x00)
            // Left arrow
            memPtr -= 0x100;
        if(key == 0x50)
            // Down arrow
            memPtr += 0x1000;
        if(key == 0x48 && (int) memPtr - 0x1000 >= 0x00)
            // Up arrow
            memPtr -= 0x1000;
        if(key == 0x2A)
            asciiFlag = !asciiFlag;
    }
}

void open_syslogs(void)
{
    kscreen::clear();
    kscreen::print_string(kernel_read_logs());

    kps2::read_ascii();
}

void open_debugger(void)
{
    kscreen::clear();

    uint32 res;

    kscreen::outargs.cursor_x = 0;
    kscreen::outargs.cursor_y = 0;

    kscreen::outargs.print_color = SCREEN_INVERTCOLOR;
    kscreen::print_string("                                    Debugger                                    ");
    kscreen::outargs.print_color = SCREEN_STDCOLOR;

    // EBP ---------------------------------
    kscreen::outargs.cursor_x = 2;
    kscreen::outargs.cursor_y = 2;
    kscreen::print_string("EBP: 0x");

    __asm__("mov %%ebp, %%edx" : "=d" (res));
    kconsole::print_hex(res, 8);

    // ESP ---------------------------------
    kscreen::outargs.cursor_x = 22;
    kscreen::outargs.cursor_y = 2;
    kscreen::print_string("ESP: 0x");

    __asm__("mov %%esp, %%edx" : "=d" (res));
    kconsole::print_hex(res, 8);

    // CS ---------------------------------
    kscreen::outargs.cursor_x = 42;
    kscreen::outargs.cursor_y = 2;
    kscreen::print_string("CS: 0x");

    __asm__("mov %%cs, %%edx" : "=d" (res));
    kconsole::print_hex(res, 8);

    // DS ---------------------------------
    kscreen::outargs.cursor_x = 62;
    kscreen::outargs.cursor_y = 2;
    kscreen::print_string("DS: 0x");

    __asm__("mov %%ds, %%edx" : "=d" (res));
    kconsole::print_hex(res, 8);

    // ES ---------------------------------
    kscreen::outargs.cursor_x = 2;
    kscreen::outargs.cursor_y = 4;
    kscreen::print_string("ES: 0x");

    __asm__("mov %%es, %%edx" : "=d" (res));
    kconsole::print_hex(res, 8);

    // GS ---------------------------------
    kscreen::outargs.cursor_x = 22;
    kscreen::outargs.cursor_y = 4;
    kscreen::print_string("GS: 0x");

    __asm__("mov %%gs, %%edx" : "=d" (res));
    kconsole::print_hex(res, 8);

    // FS ---------------------------------
    kscreen::outargs.cursor_x = 42;
    kscreen::outargs.cursor_y = 4;
    kscreen::print_string("FS: 0x");

    __asm__("mov %%fs, %%edx" : "=d" (res));
    kconsole::print_hex(res, 8);

    // SS ---------------------------------
    kscreen::outargs.cursor_x = 62;
    kscreen::outargs.cursor_y = 4;
    kscreen::print_string("SS: 0x");

    __asm__("mov %%ss, %%edx" : "=d" (res));
    kconsole::print_hex(res, 8);

    // CR0 --------------------------------
    kscreen::outargs.cursor_x = 2;
    kscreen::outargs.cursor_y = 6;
    kscreen::print_string("CR0: 0x");

    __asm__("mov %%cr0, %%edx" : "=d" (res));
    kconsole::print_hex(res, 8);

    // CR2 --------------------------------
    kscreen::outargs.cursor_x = 22;
    kscreen::outargs.cursor_y = 6;
    kscreen::print_string("CR2: 0x");

    __asm__("mov %%cr2, %%edx" : "=d" (res));
    kconsole::print_hex(res, 8);

    // CR3 --------------------------------
    kscreen::outargs.cursor_x = 42;
    kscreen::outargs.cursor_y = 6;
    kscreen::print_string("CR3: 0x");

    __asm__("mov %%cr3, %%edx" : "=d" (res));
    kconsole::print_hex(res, 8);

    // CR4 --------------------------------
    kscreen::outargs.cursor_x = 62;
    kscreen::outargs.cursor_y = 6;
    kscreen::print_string("CR4: 0x");

    __asm__("mov %%cr4, %%edx" : "=d" (res));
    kconsole::print_hex(res, 8);

    kscreen::outargs.cursor_x = 2;
    kscreen::outargs.cursor_y = 8;
    kscreen::print_string("----------------------------------------------------------------------------");

    // System memory ----------------------
    kscreen::outargs.cursor_x = 2;
    kscreen::outargs.cursor_y = 10;
    kscreen::print_string("Memory allocated for the system: ");

    uint32 systemMemory = kheap::get_system_mem_size();
    if(systemMemory / 1048576 > 0)
    {
        kconsole::printf("%d MB.", systemMemory / 1048576);
    } 
    else if(systemMemory / 1024 > 0)
    {
        kconsole::printf("%d KB.", systemMemory / 1024);
    }
    else
    {
        kconsole::printf("%d B.", systemMemory);
    }

    // Heap -------------------------------
    kscreen::outargs.cursor_x = 2;
    kscreen::outargs.cursor_y = 12;
    kscreen::print_string("Heap allocated: ");

    uint32 allocatedHeap = kheap::get_allocated_size();
    if(allocatedHeap / 1048576 > 0) 
    {
        kconsole::printf("%d MB.", allocatedHeap / 1048576);
    }
    else if(allocatedHeap / 1024 > 0)
    {
        kconsole::printf("%d KB.", allocatedHeap / 1024);
    }
    else
    {
        kconsole::printf("%d B.", allocatedHeap);
    }

    // Heap memory location ---------------
    kscreen::outargs.cursor_x = 2;
    kscreen::outargs.cursor_y = 14;
    kscreen::print_string("Heap located at: 0x");
    kconsole::print_hex((uint32) kheap::get_location_ptr(), 8);

    while(true)
    {
        uint8 key = kps2::read_scancode(true);
        if(key == 0x01) return;
    }
}

void print_systemcpu(void) 
{
    char cpuinfo_buffer[49];
    
	// registers[0] -> eax
	// registers[1] -> ebx
	// registers[2] -> ecx
	// registers[3] -> edx
	int registers[4];

	int cpuid_addr = 0x80000002;
	
    for(int i = 0; i < 3; i++) 
    {
    	cpuid_addr += i;
        __asm__("cpuid" : "=a"(registers[0]), "=b"(registers[1]), "=c"(registers[2]), "=d"(registers[3]) : "a"(cpuid_addr));
        
        for(int j = 0; j < 4; j++)
            ((int*)(cpuinfo_buffer + i * 16))[j] = registers[j];
    }

    cpuinfo_buffer[48] = '\0';
    
    kscreen::print_string(cpuinfo_buffer);
    kscreen::print_string("\n\n");
}
