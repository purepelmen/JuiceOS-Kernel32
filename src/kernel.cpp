#include "drivers/screen.hpp"
#include "drivers/ps2.hpp"

#include "kernel.hpp"
#include "heap.hpp"

// Buffer of the system log
static char systemLogBuffer[2048];
ScreenDriver screen;
Ps2 ps2;

void initializeKernel()
{
    screen.initialize();
    ps2.initialize();

    screen.clear();
    InitializeHeap();
    screen.enableCursor(0xE, 0xF);

    PrintLog("Kernel initialization completed.\n");
}

extern "C" void kernel_main()
{
    initializeKernel();
    OpenMenu();
}

void OpenConsole(void)
{
    
    screen.clear();

    while(true)
    {
        screen.printColor = STANDART_CONSOLE_PREFIX_COLOR;
        screen << "PC:>>";
        screen.printColor = STANDART_SCREEN_COLOR;

        string command = ReadString();
        command.toLower(command);
        screen << 0xA;

        if(command.length() == 0)
        {
            // If nothing was printed
            continue;
        }

        if(command.compare("hello"))
        {
            screen << "Helloooo :)\n\n";
            continue;
        }

        // TO FIX: This not working (all just hangs) 
        if(command.compare("reboot"))
        {
            screen << "This command doesn't work for now. It will be fixed soon.\n\n";
            continue;
        }

        if(command.compare("cls"))
        {
            screen.clear();
            continue;
        }

        if(command.compare("system"))
        {
            screen << "JuiceOS Kernel32 v" KERNEL_VERSION "\n\n";
            continue;
        }

        if(command.compare("exit"))
        {
            return;
        }

        if(command.compare("ascii"))
        {
            screen << "Type any char.\n";

            uint8 key = ps2.readAscii();
            screen << "ASCII code of typed key is: 0x";
            PrintByteAsString(key);
            screen << "\nIt displays as: ";

            screen << key;
            screen << "\n\n";
            continue;
        }

        if(command.compare("memdump"))
        {
            OpenMemoryDumper();
            screen.clear();
            continue;
        }

        if(command.compare("help"))
        {
            screen << "ASCII - Print hex representation of a typed char.\n";
            screen << "CLS - Clear the console.\n";
            screen << "EXIT - Quit from console to OS menu.\n";
            screen << "HELP - Print this message.\n";
            screen << "HELLO - Test command that say hello to you.\n";
            screen << "MEMDUMP - Open Memory dumper.\n";
            screen << "REBOOT - Reboot your PC.\n";
            screen << "SCANTEST - Print scancode of every pressed key.\n";
            screen << "SYSTEM - Print system information.\n\n";
            continue;
        }

        if(command.compare("scantest"))
        {
            ps2.getScancode(false);

            while(true)
            {
                uint8 scancode = ps2.getScancode(false);

                screen << "0x";
                PrintByteAsString(scancode);
                screen << "\n";
                if(scancode == 0x81) break;
            }
            
            screen << '\n';
            continue;
        }

        if(command.compare("\x1B"))
            return;
        
        screen << "Unknown command.\n\n";
    }
}

void OpenMenu(void)
{
    screen.clear();
    const uint8 NON_SELECTED_COLOR = STANDART_SCREEN_COLOR;
    const uint8 SELECTED_COLOR = STANDART_INVERTED_SCREEN_COLOR;
    const uint8 ITEMS_AMOUNT = 3;         // Zero means - 1

    uint8 currentPosition = 0;
    while(true)
    {
        screen.printColor = NON_SELECTED_COLOR;

        screen.cursorX = 25;
        screen.cursorY = 1;
        screen << "Juice OS v" KERNEL_VERSION " Menu";

        screen.cursorX = 6;
        screen.cursorY = 3;
        if(currentPosition == 0)
            screen.printColor = SELECTED_COLOR;
        else 
            screen.printColor = NON_SELECTED_COLOR;
        screen << "Open console";

        screen.cursorX = 6;
        screen.cursorY = 4;
        if(currentPosition == 1)
            screen.printColor = SELECTED_COLOR;
        else 
            screen.printColor = NON_SELECTED_COLOR;
        screen << "Memory dumper";

        screen.cursorX = 6;
        screen.cursorY = 5;
        if(currentPosition == 2)
            screen.printColor = SELECTED_COLOR;
        else 
            screen.printColor = NON_SELECTED_COLOR;
        screen << "System logs";

        screen.cursorX = 6;
        screen.cursorY = 6;
        if(currentPosition == 3)
            screen.printColor = SELECTED_COLOR;
        else 
            screen.printColor = NON_SELECTED_COLOR;
        screen << "Debug";

        // Getting input
        uint8 key = ps2.getScancode(true);
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
            screen.printColor = NON_SELECTED_COLOR;
            if(currentPosition == 0)
            {
                OpenConsole();
                screen.clear();
            }
            if(currentPosition == 1)
            {
                OpenMemoryDumper();
                screen.clear();
            }
            if(currentPosition == 2)
            {
                OpenSystemLogs();
                screen.clear();
            }
            if(currentPosition == 3)
            {
                OpenDebug();
                screen.clear();
            }
        }
    }
}

void OpenMemoryDumper(void)
{
    uint8* memPtr = (uint8*) 0x0;
    uint8 asciiFlag = false;
    screen.clear();

    while(true)
    {
        screen.printColor = STANDART_INVERTED_SCREEN_COLOR;
        screen << "                             Juice OS Memory Dumper                             ";

        screen.printColor = STANDART_SCREEN_COLOR;
        screen.cursorX = 0;
        screen.cursorY = 2;

        screen << "    Offset: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F";

        screen.cursorX = 0;
        screen.cursorY = 4;

        for(int i = 0; i < 256; i++)
        {
            if(asciiFlag)
            {
                screen << "0x";
                PrintIntAsString((uint32) memPtr + i);
                screen << ": ";

                for(int ii = 0; ii < 16; ii++)
                {
                    screen.printColor = 0x07;
                    screen.printChar_noupdates(memPtr[i + ii]);
                    screen.updateCursor();
                    screen.printColor = STANDART_SCREEN_COLOR;
                    screen << "  ";
                }

                i += 15;
                screen << '\n';
            } 
            else
            {
                screen <<"0x";
                PrintIntAsString((uint32) memPtr + i);
                screen << ": ";

                for(int ii = 0; ii < 16; ii++)
                {
                    screen.printColor = 0x07;
                    PrintByteAsString(memPtr[i + ii]);
                    screen.printColor = STANDART_SCREEN_COLOR;
                    screen << ' ';
                }

                i += 15;
                screen << '\n';
            }
        }

        screen.printColor = STANDART_INVERTED_SCREEN_COLOR;
        screen.cursorX = 0;
        screen.cursorY = 24;
        screen << "Dump: 0x";
        PrintIntAsString((uint32) memPtr);
        screen << " - 0x";
        PrintIntAsString((uint32) memPtr + 255);
        screen << " | ASCII Flag = ";
        if(asciiFlag)
            screen << "ON ";
        else
            screen << "OFF";
        screen.printString_noupdates("                                     ");

        screen.cursorX = 0;
        screen.cursorY = 0;
        screen.updateCursor();

        uint8 key = ps2.getScancode(true);
        if(key == 0x01)
            break;
        if(key == 0x4D)
            // Right arrow
            memPtr += 0x100;
        if(key == 0x4B)
            // Left arrow
            memPtr -= 0x100;
        if(key == 0x50)
            // Down arrow
            memPtr += 0x1000;
        if(key == 0x48)
            // Up arrow
            memPtr -= 0x1000;
        if(key == 0x2A)
            asciiFlag = !asciiFlag;
    }
}

void PrintLog(string str)
{
    if(string(systemLogBuffer).length() + str.length() > 2046) return;
    string(systemLogBuffer).concat(str);
}

void OpenSystemLogs(void)
{
    screen.clear();
    screen << string(systemLogBuffer);
    ps2.readAscii();
}

void OpenDebug(void)
{
    screen.clear();

    uint32 res;

    screen.cursorX = 0;
    screen.cursorY = 0;

    screen.printColor = STANDART_INVERTED_SCREEN_COLOR;
    screen << "                                    Debugger                                    ";
    screen.printColor = STANDART_SCREEN_COLOR;

    // EBP ---------------------------------
    screen.cursorX = 2;
    screen.cursorY = 2;
    screen << "EBP: 0x";

    __asm__("mov %%ebp, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // ESP ---------------------------------
    screen.cursorX = 22;
    screen.cursorY = 2;
    screen << "ESP: 0x";

    __asm__("mov %%esp, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // CS ---------------------------------
    screen.cursorX = 42;
    screen.cursorY = 2;
    screen << "CS: 0x";

    __asm__("mov %%cs, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // DS ---------------------------------
    screen.cursorX = 62;
    screen.cursorY = 2;
    screen << "DS: 0x";

    __asm__("mov %%ds, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // ES ---------------------------------
    screen.cursorX = 2;
    screen.cursorY = 4;
    screen << "ES: 0x";

    __asm__("mov %%es, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // GS ---------------------------------
    screen.cursorX = 22;
    screen.cursorY = 4;
    screen << "GS: 0x";

    __asm__("mov %%gs, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // FS ---------------------------------
    screen.cursorX = 42;
    screen.cursorY = 4;
    screen << "FS: 0x";

    __asm__("mov %%fs, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // SS ---------------------------------
    screen.cursorX = 62;
    screen.cursorY = 4;
    screen << "SS: 0x";

    __asm__("mov %%ss, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // CR0 --------------------------------
    screen.cursorX = 2;
    screen.cursorY = 6;
    screen << "CR0: 0x";

    __asm__("mov %%cr0, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // CR2 --------------------------------
    screen.cursorX = 22;
    screen.cursorY = 6;
    screen << "CR2: 0x";

    __asm__("mov %%cr2, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // CR3 --------------------------------
    screen.cursorX = 42;
    screen.cursorY = 6;
    screen << "CR3: 0x";

    __asm__("mov %%cr3, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // CR4 --------------------------------
    screen.cursorX = 62;
    screen.cursorY = 6;
    screen << "CR4: 0x";

    __asm__("mov %%cr4, %%edx" : "=d" (res));
    PrintIntAsString(res);

    screen.cursorX = 2;
    screen.cursorY = 8;
    screen << "----------------------------------------------------------------------------";

    // System memory ----------------------
    screen.cursorX = 2;
    screen.cursorY = 10;
    screen << "Memory allocated for the system: ";

    uint32 systemMemory = heapStartValue - 0x100000;
    if(systemMemory / 1048576 > 0)
    {
        PrintNum(systemMemory / 1048576);
        screen << " MB.";
    } 
    else if(systemMemory / 1024 > 0)
    {
        PrintNum(systemMemory / 1024);
        screen << " KB.";
    }
    else
    {
        PrintNum(systemMemory);
        screen << " B.";
    }

    // Heap -------------------------------
    screen.cursorX = 2;
    screen.cursorY = 12;
    screen << "Heap allocated: ";

    uint32 allocatedHeap = currentHeapValue - heapStartValue;
    if(allocatedHeap / 1048576 > 0) 
    {
        PrintNum(allocatedHeap / 1048576);
        screen << " MB.";
    }
    else if(allocatedHeap / 1024 > 0)
    {
        PrintNum(allocatedHeap / 1024);
        screen << " KB.";
    }
    else
    {
        PrintNum(allocatedHeap);
        screen << " B.";
    }

    // Heap memory location ---------------
    screen.cursorX = 2;
    screen.cursorY = 14;
    screen << "Heap located at: 0x";
    PrintIntAsString(heapStartValue);

    while(true)
    {
        uint8 key = ps2.getCurrentKey();
        if(key == 0x01) return;
    }
}
