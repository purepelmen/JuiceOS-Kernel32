#include "shell.h"
#include "heap.h"
#include "console.h"
#include "kernel.h"
#include "cpuid.h"

#include "drivers/screen.h"
// #include "drivers/ahci.h"
#include "drivers/pit.h"
#include "drivers/ps2.h"
#include "drivers/pci.h"
#include "drivers/ide.h"
#include "drivers/storage.h"

#include "filesystems/fat16.h"

kfat::FAT16* fat_pt2;
static unsigned selectedVolume = 0;
static char cwd[kstorage::MAX_FILENAME_SIZE] = "";

static void console_handle(string command, int argc, char** argv, bool* shouldContinue);

static void open_memdumper(void);
static void open_syslogs(void);

static void print_systemcpu(void);

static void parse_args(const char* source, char* outContent, int maxOutArgv, char** outArgv, int& outArgc)
{
    if (maxOutArgv < 1)
        return;
    
    char* curArg = nullptr;
    outArgc = 0;

    // Allocating space for nullptr at outArgv[maxOutArgv].
    maxOutArgv -= 1;
    
    bool hasOpenedQuotes = false;
    int destIdx = 0;
    for (int i = 0; source[i] != 0x0; i++)
    {
        char ch = source[i];
        if (ch == ' ' && !hasOpenedQuotes)
        {
            if (curArg != nullptr)
            {
                if (outArgc >= maxOutArgv)
                    break;
                
                outContent[destIdx++] = 0x0;
                outArgv[outArgc++] = curArg;

                curArg = nullptr;
            }

            continue;
        }
        else if (ch == '"')
        {
            hasOpenedQuotes = !hasOpenedQuotes;
            continue;
        }

        if (curArg == nullptr)
            curArg = outContent + destIdx;

        if (ch == '\\')
        {
            i += 1;
            ch = source[i];

            if (ch == '"' || ch == '\\')
                outContent[destIdx++] = ch;
            else if (ch == 0x0)
                break;
            
            continue;
        }

        outContent[destIdx++] = ch;
    }

    if (curArg != nullptr && outArgc < maxOutArgv)
    {
        outContent[destIdx++] = 0x0;
        outArgv[outArgc++] = curArg;

        curArg = nullptr;
    }

    outArgv[outArgc] = nullptr;
}

static char consoleParsedArgvContent[kconsole::INPUT_MAXCHARS + 1]; 

const int MAX_CONSOLE_PARSED_ARGV = 60;
static char* consoleParsedArgv[MAX_CONSOLE_PARSED_ARGV];

class Paginator
{
private:
    int counter = 0;

public:
    bool iterate()
    {
        counter++;
        if (counter >= kscreen::height() - 2)
        {
            kscreen::outargs.print_color = 0x08;
            kscreen::print("Any key to get more, ESC - exit.");

            uint8 ch = kps2::read_ascii();

            kscreen::outargs.cursor_x = 0;
            kscreen::print("                                ");

            if (ch == '\x1B')
                return false;

            counter = 0;
        }

        return true;
    }

    static void viewLongText(const char* text)
    {
        const char* segmentStart = text;
        char lastChar;

        Paginator paginator{};
        do
        {
            size_t size = 0;
            while (segmentStart[size] != '\n' && segmentStart[size] != 0x0)
                size++;
            
            lastChar = segmentStart[size];
            size++; // Include the \n or null term.

            kconsole::print(segmentStart, size);
            segmentStart += size;
        }
        while (paginator.iterate() && lastChar != 0x0);
    }
};

static void set_cwd(const char* newCwd)
{
    char tempFallback[kstorage::MAX_FILENAME_SIZE];
    if (newCwd[0] == '/')
    {
        kstorage::normalize_path(newCwd, tempFallback, sizeof(tempFallback));
    }
    else
    {
        char pathBuildBuff[kstorage::MAX_FILENAME_SIZE];
    
        int i = 0;
        i += strlcpy(cwd, pathBuildBuff, sizeof(pathBuildBuff));
        i += strlcpy("/", pathBuildBuff + i, sizeof(pathBuildBuff) - i);
        i += strlcpy(newCwd, pathBuildBuff + i, sizeof(pathBuildBuff) - i);
    
        kstorage::normalize_path(pathBuildBuff, tempFallback, sizeof(tempFallback));
    }
    
    kstorage::FileState state;
    bool exists = kstorage::volumes[selectedVolume]->resolve_path(tempFallback, state);

    if (!exists)
    {
        kconsole::printf("No such directory.\n");
        return;
    }

    strcpy(tempFallback, cwd);
}

static const char* get_fullpath(const char* relPath)
{
    char concatPath[kstorage::MAX_FILENAME_SIZE];
    static char internal[kstorage::MAX_FILENAME_SIZE];
    
    int i = 0;
    if (relPath[0] == '/')
    {
        i += kstorage::normalize_path(relPath, internal + i, kstorage::MAX_FILENAME_SIZE - i);
        return internal;
    }
    
    i += strlcpy(cwd, concatPath, kstorage::MAX_FILENAME_SIZE);
    i += strlcpy("/", concatPath + i, kstorage::MAX_FILENAME_SIZE - i);
    i += strlcpy(relPath, concatPath + i, kstorage::MAX_FILENAME_SIZE - i);

    kstorage::normalize_path(concatPath, internal, kstorage::MAX_FILENAME_SIZE);
    return internal;
}

void kshell::open_console(void)
{
    kconsole::clear();

    bool shouldContinue = true;
    while (shouldContinue)
    {
        kconsole::cursor.color = 0x02;
        kconsole::printf("PC:%s>>", cwd);
        kconsole::cursor.color = KSCREEN_STDCOLOR;

        string input = kconsole::read_string();
        
        int argc;
        parse_args(input.ptr(), consoleParsedArgvContent, MAX_CONSOLE_PARSED_ARGV, consoleParsedArgv, argc);

        if (argc == 0)
            continue;

        string command = consoleParsedArgv[0];
        command.to_lower(command);

        if(command.length() == 0)
        {
            // If nothing was printed
            continue;
        }

        if(command == "\x1B")
            return;

        console_handle(command, argc - 1, ((char**)consoleParsedArgv) + 1, &shouldContinue);
        kconsole::printc('\n');
    }
}

void console_handle(string command, int argc, char** argv, bool* shouldContinue)
{
    if(command == "hello")
    {
        kconsole::print("Helloooo :)\n");
        return;
    }

    // Awaiting IDT re-implementation
    // TO FIX: This not working (all just hangs) 
    if(command == "reboot")
    {
        kconsole::print("This command doesn't work for now. It will be fixed soon.\n");
        return;
    }

    if(command == "cls")
    {
        kconsole::clear();
        return;
    }

    if(command == "system")
    {
        kconsole::print("JuiceOS Kernel32 v" KERNEL_VERSION "\n");
        print_systemcpu();

        return;
    }

    if(command == "systemcpu")
    {
        print_systemcpu();
        return;
    }

    if (command == "exit")
    {
        *shouldContinue = false;
        return;
    }

    if(command == "ascii")
    {
        kconsole::print("Type any char.\n");

        uint8 key = kps2::read_ascii();
        kconsole::print("ASCII code of typed key is: 0x");
        kconsole::print_hex(key, 2);
        kconsole::print("\nIt displays as: ");

        kconsole::printc(key);
        kconsole::print("\n");
        return;
    }

    if(command == "memdump")
    {
        open_memdumper();
        kconsole::clear();
        return;
    }

    if (command == "dbginfo")
    {
        kshell::open_debugger();
        kconsole::clear();
        return;
    }

    if (command == "heapdbg")
    {
        kconsole::printf("=== Free blocks of heap ===\n");
        kheap::print_alloc_free_blocks();
        return;
    }

    if(command == "help")
    {
        const char* helpMsg = 
            "ASCII - Print hex representation of a typed char.\n"
            "CLS - Clear the console.\n"
            "EXIT - Quit from console to OS menu.\n"
            "HELP - Print this message.\n"
            "HELLO - Test command that say hello to you.\n"
            "MEMDUMP - Open Memory dumper.\n"
            "DBGINFO - Open Debugger.\n"
            "REBOOT - Reboot your PC.\n"
            "SCANTEST - Print scancode of every pressed key.\n"
            "SYSTEM - Print system information.\n"
            "SYSTEMCPU - Print information about CPU.\n"
            "PCI - Print all PCI devices.\n"
            "IDEDEV - Print all ATA devices from IDE driver.\n"
            "IDERD - Read first sector from the first IDE device.\n"
            "VOLUMELIST - View all mounted volumes.\n"
            "SETVOLUME - Set current volume for the shell.\n"
            "DIR / LS - Print directory contents at path.\n"
            "CD - Change current working directory.\n"
            "READFILE - Print contents of the file.\n"
            "PRINTARGS - Prints every argument specified to this command.\n";
        
        // kconsole::print("AHCIVER - Print AHCI specification version.\n");
        // kconsole::print("AHCIDEV - Print all AHCI ports and connected devices.\n");
        // kconsole::print("AHCIRD - Read first sector from AHCI port #0.\n");

        Paginator::viewLongText(helpMsg);
        return;
    }

    if(command == "scantest")
    {
        kps2::read_scancode(false);

        while(true)
        {
            uint8 scancode = kps2::read_scancode(false);

            kconsole::print("0x");
            kconsole::print_hex(scancode, 2);

            kconsole::printc('\n');
            if(scancode == 0x81) break;
        }
        
        return;
    }

    if(command == "pci")
    {
        for(size_t i = 0; i < kpci::devicesCount; i++)
        {
            kpci::pci_device* device = &kpci::devices[i];
            kconsole::printf("PCI Device (Bus/Slot/Fun: %d,%d,%d) = Class/Subclass: %d,%d\n", 
                device->bus, device->slot, device->function, device->classid, device->subclass);
        }

        return;
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

        return;
    }

    if (command == "iderd")
    {
        kconsole::printf("ATA device number: ");
        
        unsigned deviceNum = str_to_uint(kconsole::read_string().ptr());
        if (deviceNum >= kide::deviceCount)
        {
            kconsole::printf("No ATA device found, there's only %d of them.\n", kide::deviceCount);
            return;
        }

        kide::AtaDevice* device = &kide::devices[deviceNum];

        uint8 buff[512];
        if (kide::ata_read_sector(device->addr, device->isSlave, 0, 1, (uint16*)buff))
        {
            for(int i = 0; i < 256; i++)
            {
                kconsole::print_hex(buff[i], 2);
                kconsole::printc(' ');
            }

            kconsole::printc('\n');
        }
        else
        {
            kconsole::printf("Failed to read test data from an ATA device.\n");
        }

        return;
    }

    if(command == "fatinit")
    {
        kconsole::printf("Initializing FAT on partition #2\n");
        fat_pt2 = kheap::create_new<kfat::FAT16>(0);
        fat_pt2->init(1);

        return;
    }

    if(command == "fatdir")
    {
        kconsole::printf("K32 Partition files\n\n");
        fat_pt2->dir(0);

        return;
    }

    if (command == "volumelist")
    {
        for (int i = 0; i < kstorage::volumeCount; i++)
        {
            kstorage::FileSystem* volume = kstorage::volumes[i];
            kconsole::printf("[Volume #%d] '%s', size=%d KB\n", i, volume->get_name(), volume->get_size() / 1024);
        }

        return;
    }

    if (command == "setvolume")
    {
        const char* input = argv[0];
        if (argc < 1)
        {
            kconsole::printf("Volume number: ");
            input = kconsole::read_string().ptr();
        }
        
        unsigned volumeNumber = str_to_uint(input);
        if (volumeNumber >= kstorage::volumeCount)
        {
            kconsole::printf("No such volume found, there's only %d of them.\n", kstorage::volumeCount);
            return;
        }

        selectedVolume = volumeNumber;
        return;
    }

    if (command == "cd")
    {
        if (argc < 1)
            return;

        set_cwd(argv[0]);
        return;
    }
    
    if (command == "dir" || command == "ls")
    {
        if (selectedVolume >= kstorage::volumeCount)
        {
            kconsole::printf("Currently selected volume (#%d) is not mounted.\n", selectedVolume);
            return;
        }

        struct ls_state
        {
            Paginator paginator{};
        } state;

        const char* path = cwd;
        if (argc >= 1)
        {
            path = get_fullpath(argv[0]);
        }

        kstorage::FileSystem* fileSystem = kstorage::volumes[selectedVolume];
        fileSystem->read_dir(path, [](void* context, kstorage::DirEntry& entry)
        {
            ls_state* state = (ls_state*)context;

            const char* typeStr = entry.type == kstorage::DirEntryType::DIRECTORY ? "<D>  " : "     ";
            kconsole::print(typeStr);
            kconsole::print(" ");

            if (entry.type == kstorage::DirEntryType::DIRECTORY)
                kconsole::cursor.color = 0x09;
            kconsole::print(entry.name);
            kconsole::cursor.color = KSCREEN_STDCOLOR;

            if (entry.type == kstorage::DirEntryType::FILE)
            {
                kconsole::cursor.posX = 45;
                kconsole::printf("%db", entry.size);
            }
            kconsole::printc('\n');

            return state->paginator.iterate();
        }, &state);

        return;
    }

    if (command == "readfile")
    {
        if (selectedVolume >= kstorage::volumeCount)
        {
            kconsole::printf("Currently selected volume (#%d) is not mounted.\n", selectedVolume);
            return;
        }

        const char* path = cwd;
        if (argc >= 1)
        {
            path = get_fullpath(argv[0]);
        }
        
        kstorage::FileSystem* fileSystem = kstorage::volumes[selectedVolume];

        kstorage::FileState fileState;
        if (!fileSystem->resolve_path(path, fileState))
        {
            kconsole::printf("No such file found.\n");
            return;
        }

        char buff[256];
        while (true)
        {
            size_t actuallyRead = fileSystem->read(fileState, buff, 256);
            kconsole::print(buff, actuallyRead);

            if (fileState.is_eof())
                break;

            uint8 key = kps2::read_ascii();
            if (key == '\x1B')
                break;
        }

        kconsole::printc('\n');
        return;
    }

    if (command == "printargs")
    {
        for (char** argIter = argv; *argIter != nullptr; argIter++)
        {
            kconsole::printf("'%s'\n", *argIter);
        }
        
        return;
    }
    
    kconsole::print("Unknown command.\n");
}

static int make_menu(const char** items, size_t itemsAmount, int* currentPosition)
{
    const uint8 NON_SELECTED_COLOR = KSCREEN_STDCOLOR;
    const uint8 SELECTED_COLOR = KSCREEN_INVERTCOLOR;

    uint8 baseX = kscreen::outargs.cursor_x;
    for (int i = 0; i < itemsAmount; i++)
    {
        kscreen::outargs.print_color = *currentPosition == i ? SELECTED_COLOR : NON_SELECTED_COLOR;
        kscreen::outargs.cursor_x = baseX;
        kscreen::print(items[i]);

        kscreen::outargs.cursor_y++;
    }

    kscreen::outargs.print_color = NON_SELECTED_COLOR;

    // Checking input.
    uint8 key = kps2::read_scancode(true);

    if(key == 0x1C)
        return *currentPosition;

    if(key == 0x48)
    {
        (*currentPosition)--;
        if (*currentPosition < 0)
            *currentPosition = itemsAmount - 1;
    }
    else if(key == 0x50)
    {
        (*currentPosition)++;
        if (*currentPosition == itemsAmount)
            *currentPosition = 0;
    }

    return -1;
}

void kshell::open_menu(void)
{
    kconsole::clear();
    const char* menuItems[] =
    { 
        "Open console",
        "Memory dumper",
        "System logs",
        "Debug"
    };

    int menuPos = 0;
    while (true)
    {
        kscreen::outargs.print_color = KSCREEN_STDCOLOR;

        kscreen::outargs.cursor_x = 25;
        kscreen::outargs.cursor_y = 1;
        kscreen::print("Juice OS v" KERNEL_VERSION " Menu");

        kscreen::outargs.cursor_x = 6;
        kscreen::outargs.cursor_y = 3;

        if (make_menu(menuItems, 4, &menuPos) != -1)
        {
            if (menuPos == 0)
            {
                kshell::open_console();
                kconsole::clear();
            }
            else if (menuPos == 1)
            {
                open_memdumper();
                kconsole::clear();
            }
            else if (menuPos == 2)
            {
                open_syslogs();
                kconsole::clear();
            }
            else if (menuPos == 3)
            {
                open_debugger();
                kconsole::clear();
            }
        }
    }
}

void screen_printf(const char* str, ...)
{
    va_list args;

    va_start(args, str);
    vsprintf([](void* context, const char* portionPtr, int length) 
    {
        kscreen::print(portionPtr, length);
    }, nullptr, str, args);

    va_end(args);
}

void open_memdumper(void)
{
    uint8* memPtr = (uint8*) 0x0;
    uint8 asciiFlag = false;
    kconsole::clear();

    while(true)
    {
        kscreen::outargs.print_color = KSCREEN_INVERTCOLOR;
        kscreen::print(0, 0, "                             Juice OS Memory Dumper                             ");

        kscreen::outargs.print_color = KSCREEN_STDCOLOR;
        kscreen::print(0, 2, "    Offset: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");

        kscreen::outargs.cursor_x = 0;
        kscreen::outargs.cursor_y = 4;

        for (int i = 0; i < 256; i++)
        {
            screen_printf("0x%x: ", memPtr + i);

            if (asciiFlag)
            {
                for(int j = 0; j < 16; j++)
                {
                    kscreen::printc(memPtr[i + j]);
                    kscreen::print("  ");
                }
            } 
            else
            {
                for(int j = 0; j < 16; j++)
                {
                    kconsole::sync_scursor_coords();
                    kconsole::print_hex(memPtr[i + j], 2);
                    kscreen::printc(' ');
                }
            }

            kscreen::outargs.cursor_x = 0;
            kscreen::outargs.cursor_y++;
            
            i += 15;
        }

        kscreen::outargs.print_color = KSCREEN_INVERTCOLOR;
        kscreen::outargs.cursor_x = 0;
        kscreen::outargs.cursor_y = 24;
        screen_printf("Dump: 0x%x - 0x%x | ASCII Flag = %s                                ", memPtr, memPtr + 255, asciiFlag ? "ON " : "OFF");

        kscreen::update_hwcursor(0, 0);

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
    int pageIndex = 0;
    while (true)
    {
        kconsole::clear();

        bool isThereAreMorePages = kernel_render_logs(pageIndex);

        uint8 key = kps2::read_scancode(true);
        if (key == 0x01)
            break;

        // Down and Up arrow respectively.
        if (key == 0x50 && isThereAreMorePages)
        {
            pageIndex++;
        }
        else if (key == 0x48 && pageIndex > 0)
        {
            pageIndex--;
        }
    }
}

static void printf_size(size_t bytes)
{
    size_t conv = bytes / (1024 * 1024);
    if (conv > 0)
    {
        screen_printf("%d MB", conv);
        return;
    }

    conv = bytes / 1024;
    if (conv > 0)
    {
        screen_printf("%d KB", conv);
        return;
    }

    screen_printf("%d B", bytes);
}

void kshell::open_debugger(void)
{
    kscreen::clear();
    kscreen::update_hwcursor(0, 0);

    uint32 res;

    kscreen::outargs.print_color = KSCREEN_INVERTCOLOR;
    kscreen::print("                                    Debugger                                    ");
    kscreen::outargs.print_color = KSCREEN_STDCOLOR;

    // EBP ---------------------------------
    __asm__("mov %%ebp, %%edx" : "=d" (res));

    kscreen::outargs.cursor_x = 2;
    kscreen::outargs.cursor_y = 2;
    screen_printf("EBP: 0x%x", res);

    // ESP ---------------------------------
    __asm__("mov %%esp, %%edx" : "=d" (res));

    kscreen::outargs.cursor_x = 22;
    kscreen::outargs.cursor_y = 2;
    screen_printf("ESP: 0x%x", res);

    // CS ---------------------------------
    __asm__("mov %%cs, %%edx" : "=d" (res));

    kscreen::outargs.cursor_x = 42;
    kscreen::outargs.cursor_y = 2;
    screen_printf("CS: 0x%x", res);

    // DS ---------------------------------
    __asm__("mov %%ds, %%edx" : "=d" (res));

    kscreen::outargs.cursor_x = 62;
    kscreen::outargs.cursor_y = 2;
    screen_printf("DS: 0x%x", res);

    // ES ---------------------------------
    __asm__("mov %%es, %%edx" : "=d" (res));

    kscreen::outargs.cursor_x = 2;
    kscreen::outargs.cursor_y = 4;
    screen_printf("ES: 0x%x", res);

    // GS ---------------------------------
    __asm__("mov %%gs, %%edx" : "=d" (res));
    kscreen::outargs.cursor_x = 22;
    kscreen::outargs.cursor_y = 4;
    screen_printf("GS: 0x%x", res);

    // FS ---------------------------------
    __asm__("mov %%fs, %%edx" : "=d" (res));

    kscreen::outargs.cursor_x = 42;
    kscreen::outargs.cursor_y = 4;
    screen_printf("FS: 0x%x", res);

    // SS ---------------------------------
    __asm__("mov %%ss, %%edx" : "=d" (res));

    kscreen::outargs.cursor_x = 62;
    kscreen::outargs.cursor_y = 4;
    screen_printf("SS: 0x%x", res);

    // CR0 --------------------------------
    __asm__("mov %%cr0, %%edx" : "=d" (res));

    kscreen::outargs.cursor_x = 2;
    kscreen::outargs.cursor_y = 6;
    screen_printf("CR0: 0x%x", res);

    // CR2 --------------------------------
    __asm__("mov %%cr2, %%edx" : "=d" (res));

    kscreen::outargs.cursor_x = 22;
    kscreen::outargs.cursor_y = 6;
    screen_printf("CR2: 0x%x", res);

    // CR3 --------------------------------
    __asm__("mov %%cr3, %%edx" : "=d" (res));

    kscreen::outargs.cursor_x = 42;
    kscreen::outargs.cursor_y = 6;
    screen_printf("CR3: 0x%x", res);

    // CR4 --------------------------------
    __asm__("mov %%cr4, %%edx" : "=d" (res));
    
    kscreen::outargs.cursor_x = 62;
    kscreen::outargs.cursor_y = 6;
    screen_printf("CR4: 0x%x", res);

    kscreen::outargs.cursor_x = 2;
    kscreen::outargs.cursor_y = 8;
    kscreen::print("----------------------------------------------------------------------------");

    // System memory ----------------------
    kscreen::outargs.cursor_x = 2;
    kscreen::outargs.cursor_y = 10;
    kscreen::print("Kernel memsize: ");
    printf_size(kheap::get_kernel_memsize());
    kscreen::print(".");

    // Heap -------------------------------
    kscreen::outargs.cursor_x = 2;
    kscreen::outargs.cursor_y = 12;
    kscreen::print("Dynamic memory size: ");
    printf_size(kheap::get_allocated());
    kscreen::print(".");

    // Heap memory location ---------------
    kscreen::outargs.cursor_x = 2;
    kscreen::outargs.cursor_y = 14;
    kscreen::print("Heap free: ");
    printf_size(kheap::get_heap_free());
    kscreen::print(" / ");
    printf_size(kheap::get_heap_size());
    kscreen::print(".");

    // Stack ------------------------------
    kscreen::outargs.cursor_x = 2;
    kscreen::outargs.cursor_y = 16;
    screen_printf("Stack usage: ");
    printf_size(kheap::get_stack_usage());
    kscreen::print(".");

    while (kps2::read_scancode(true) != 0x01);
}

void print_systemcpu(void) 
{
    if (!is_cpuid_supported())
    {
        kconsole::print("No CPUID support, can't provide more info.\n");
        return;
    }

    char cpuinfo_buffer[49];
	cpuid_80000002h(cpuinfo_buffer);
    
    kconsole::print(cpuinfo_buffer);
    kconsole::print("\n");

    char vendor[13];
    kconsole::printf("The CPUID result: %d, Vendor: %s.\n", cpuid(vendor), vendor);

    cpuid_q1edx ext = cpuid_eax1().ext;

    kconsole::printf("Extensions: FPU = %d, TSC = %d, MMX = %d, MMX2 = %d, SSE = %d, V86 = %d, CR4.PGE Support = %d, PSE (4MB Page) = %d\n", 
        ext.FPU, ext.TSC, ext.MMX, ext.FXSR, ext.SSE, ext.VME, ext.PGE, ext.PSE);
}
