#include "drivers/screen.h"
#include "drivers/ps2.h"

#include "kernel.h"
#include "heap.h"

// Buffer of the system log
static uint8 systemLogBuffer[2048];

void InitializeKernel(void)
{
    ClearScreen();
    
    InitializeHeap();
    EnableCursor(0xE, 0xF);

    PrintLog("Kernel initialization completed.\n");
}

void kernel_main(void)
{
    InitializeKernel();
    OpenMenu();
}

void OpenConsole(void)
{
    ClearScreen();

    while(true)
    {
        PrintColor = STANDART_CONSOLE_PREFIX_COLOR;
        PrintString("PC:>>");
        PrintColor = STANDART_SCREEN_COLOR;

        uint8* command = ReadString();
        ToLowerCase(command, command);
        PrintChar(0xA);

        if(GetStringLength(command) == 0)
        {
            // If nothing was printed
            continue;
        }

        if(CompareString(command, "hello"))
        {
            PrintString("Helloooo :)\n\n");
            continue;
        }

        // TO FIX: This not working (all just hangs) 
        if(CompareString(command, "reboot"))
        {
            PrintString("This command doesn't work for now. It will be fixed soon.\n\n");
            continue;
        }

        if(CompareString(command, "cls"))
        {
            ClearScreen();
            continue;
        }

        if(CompareString(command, "system"))
        {
            PrintString("JuiceOS Kernel32 v" KERNEL_VERSION "\n\n");
            continue;
        }

        if(CompareString(command, "exit"))
        {
            return;
        }

        if(CompareString(command, "ascii"))
        {
            PrintString("Type any char.\n");

            uint8 key = Ps2ReadKey();
            PrintString("ASCII code of typed key is: 0x");
            PrintByteAsString(key);
            PrintString("\nIt displays as: ");

            PrintChar(key);
            PrintString("\n\n");
            continue;
        }

        if(CompareString(command, "memdump"))
        {
            OpenMemoryDumper();
            ClearScreen();
            continue;
        }

        if(CompareString(command, "help"))
        {
            PrintString("ASCII - Print hex representation of a typed char.\n");
            PrintString("CLS - Clear the console.\n");
            PrintString("EXIT - Quit from console to OS menu.\n");
            PrintString("HELP - Print this message.\n");
            PrintString("HELLO - Test command that say hello to you.\n");
            PrintString("MEMDUMP - Open Memory dumper.\n");
            PrintString("REBOOT - Reboot your PC.\n");
            PrintString("SCANTEST - Print scancode of every pressed key.\n");
            PrintString("SYSTEM - Print system information.\n\n");
            continue;
        }

        if(CompareString(command, "scantest"))
        {
            Ps2GetScancode(false);

            while(true)
            {
                uint8 scancode = Ps2GetScancode(false);

                PrintString("0x");
                PrintByteAsString(scancode);
                PrintString("\n");
                if(scancode == 0x81) break;
            }
            
            PrintChar('\n');
            continue;
        }

        if(CompareString(command, "\x1B"))
            return;
        
        PrintString("Unknown command.\n\n");
    }
}

void OpenMenu(void)
{
    ClearScreen();
    const uint8 NON_SELECTED_COLOR = STANDART_SCREEN_COLOR;
    const uint8 SELECTED_COLOR = STANDART_INVERTED_SCREEN_COLOR;
    const uint8 ITEMS_AMOUNT = 3;         // Zero means - 1

    uint8 currentPosition = 0;
    while(true)
    {
        PrintColor = NON_SELECTED_COLOR;

        cursorX = 25;
        cursorY = 1;
        PrintString("Juice OS v" KERNEL_VERSION " Menu");

        cursorX = 6;
        cursorY = 3;
        if(currentPosition == 0)
            PrintColor = SELECTED_COLOR;
        else 
            PrintColor = NON_SELECTED_COLOR;
        PrintString("Open console");

        cursorX = 6;
        cursorY = 4;
        if(currentPosition == 1)
            PrintColor = SELECTED_COLOR;
        else 
            PrintColor = NON_SELECTED_COLOR;
        PrintString("Memory dumper");

        cursorX = 6;
        cursorY = 5;
        if(currentPosition == 2)
            PrintColor = SELECTED_COLOR;
        else 
            PrintColor = NON_SELECTED_COLOR;
        PrintString("System logs");

        cursorX = 6;
        cursorY = 6;
        if(currentPosition == 3)
            PrintColor = SELECTED_COLOR;
        else 
            PrintColor = NON_SELECTED_COLOR;
        PrintString("Debug");

        // Getting input
        uint8 key = Ps2GetScancode(true);
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
            PrintColor = NON_SELECTED_COLOR;
            if(currentPosition == 0)
            {
                OpenConsole();
                ClearScreen();
            }
            if(currentPosition == 1)
            {
                OpenMemoryDumper();
                ClearScreen();
            }
            if(currentPosition == 2)
            {
                OpenSystemLogs();
                ClearScreen();
            }
            if(currentPosition == 3)
            {
                OpenDebug();
                ClearScreen();
            }
        }
    }
}

void OpenMemoryDumper(void)
{
    uint8* memPtr = (uint8*) 0x0;
    uint8 asciiFlag = false;
    ClearScreen();

    while(true)
    {
        PrintColor = STANDART_INVERTED_SCREEN_COLOR;
        PrintString("                             Juice OS Memory Dumper                             ");

        PrintColor = STANDART_SCREEN_COLOR;
        cursorX = 0;
        cursorY = 2;

        PrintString("    Offset: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");

        cursorX = 0;
        cursorY = 4;

        for(int i=0; i < 256; i++)
        {
            if(asciiFlag)
            {
                PrintString("0x");
                PrintIntAsString((uint32) memPtr + i);
                PrintString(": ");

                for(int ii=0; ii < 16; ii++)
                {
                    PrintColor = 0x07;
                    print_char_noupdates(memPtr[i + ii]);
                    UpdateCursor();
                    PrintColor = STANDART_SCREEN_COLOR;
                    PrintString("  ");
                }
                i += 15;
                PrintChar('\n');
            } 
            else
            {
                PrintString("0x");
                PrintIntAsString((uint32) memPtr + i);
                PrintString(": ");

                for(int ii=0; ii < 16; ii++)
                {
                    PrintColor = 0x07;
                    PrintByteAsString(memPtr[i + ii]);
                    PrintColor = STANDART_SCREEN_COLOR;
                    PrintChar(' ');
                }

                i += 15;
                PrintChar('\n');
            }
        }

        PrintColor = STANDART_INVERTED_SCREEN_COLOR;
        cursorX = 0;
        cursorY = 24;
        PrintString("Dump: 0x");
        PrintIntAsString((uint32) memPtr);
        PrintString(" - 0x");
        PrintIntAsString((uint32) memPtr + 255);
        PrintString(" | ASCII Flag = ");
        if(asciiFlag)
            PrintString("ON ");
        else
            PrintString("OFF");
        print_string_noupdates("                                     ");

        cursorX = 0;
        cursorY = 0;
        UpdateCursor();

        uint8 key = Ps2GetScancode(true);
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

void PrintLog(uint8* str)
{
    if(GetStringLength((uint8*) &systemLogBuffer) + GetStringLength(str) > 2046) return;
    ConcatString((uint8*) &systemLogBuffer, str);
}

void OpenSystemLogs(void)
{
    ClearScreen();
    PrintString((uint8*) &systemLogBuffer);
    Ps2ReadKey();
}

void OpenDebug(void)
{
    ClearScreen();

    uint32 res;

    cursorX = 0;
    cursorY = 0;

    PrintColor = STANDART_INVERTED_SCREEN_COLOR;
    PrintString("                                    Debugger                                    ");
    PrintColor = STANDART_SCREEN_COLOR;

    // EBP ---------------------------------
    cursorX = 2;
    cursorY = 2;
    PrintString("EBP: 0x");

    __asm__("mov %%ebp, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // ESP ---------------------------------
    cursorX = 22;
    cursorY = 2;
    PrintString("ESP: 0x");

    __asm__("mov %%esp, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // CS ---------------------------------
    cursorX = 42;
    cursorY = 2;
    PrintString("CS: 0x");

    __asm__("mov %%cs, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // DS ---------------------------------
    cursorX = 62;
    cursorY = 2;
    PrintString("DS: 0x");

    __asm__("mov %%ds, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // ES ---------------------------------
    cursorX = 2;
    cursorY = 4;
    PrintString("ES: 0x");

    __asm__("mov %%es, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // GS ---------------------------------
    cursorX = 22;
    cursorY = 4;
    PrintString("GS: 0x");

    __asm__("mov %%gs, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // FS ---------------------------------
    cursorX = 42;
    cursorY = 4;
    PrintString("FS: 0x");

    __asm__("mov %%fs, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // SS ---------------------------------
    cursorX = 62;
    cursorY = 4;
    PrintString("SS: 0x");

    __asm__("mov %%ss, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // CR0 --------------------------------
    cursorX = 2;
    cursorY = 6;
    PrintString("CR0: 0x");

    __asm__("mov %%cr0, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // CR2 --------------------------------
    cursorX = 22;
    cursorY = 6;
    PrintString("CR2: 0x");

    __asm__("mov %%cr2, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // CR3 --------------------------------
    cursorX = 42;
    cursorY = 6;
    PrintString("CR3: 0x");

    __asm__("mov %%cr3, %%edx" : "=d" (res));
    PrintIntAsString(res);

    // CR4 --------------------------------
    cursorX = 62;
    cursorY = 6;
    PrintString("CR4: 0x");

    __asm__("mov %%cr4, %%edx" : "=d" (res));
    PrintIntAsString(res);

    cursorX = 2;
    cursorY = 8;
    PrintString("----------------------------------------------------------------------------");

    // System memory ----------------------
    cursorX = 2;
    cursorY = 10;
    PrintString("Memory allocated for the system: ");

    uint32 systemMemory = heapStartValue - 0x100000;
    if(systemMemory / 1048576 > 0)
    {
        PrintNum(systemMemory / 1048576);
        PrintString(" MB.");
    } 
    else if(systemMemory / 1024 > 0)
    {
        PrintNum(systemMemory / 1024);
        PrintString(" KB.");
    }
    else
    {
        PrintNum(systemMemory);
        PrintString(" B.");
    }

    // Heap -------------------------------
    cursorX = 2;
    cursorY = 12;
    PrintString("Heap allocated: ");

    uint32 allocatedHeap = currentHeapValue - heapStartValue;
    if(allocatedHeap / 1048576 > 0) 
    {
        PrintNum(allocatedHeap / 1048576);
        PrintString(" MB.");
    }
    else if(allocatedHeap / 1024 > 0)
    {
        PrintNum(allocatedHeap / 1024);
        PrintString(" KB.");
    }
    else
    {
        PrintNum(allocatedHeap);
        PrintString(" B.");
    }

    // Heap memory location ---------------
    cursorX = 2;
    cursorY = 14;
    PrintString("Heap located at: 0x");
    PrintIntAsString(heapStartValue);

    while(true)
    {
        uint8 key = Ps2KeyDown();
        if(key == 0x01) return;
    }
}
