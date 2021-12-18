#include "drivers/screen.h"
#include "drivers/ps2.h"
#include "stdio.h"
#include "heap.h"

#include "isr/gdt_idt.h"
#include "kernel.h"
#include "system.h"

// Buffer of the system log
static uint8 systemLogBuffer[2048];

void InitializeKernel(void)
{
    ClearScreen();
    
    // Initialise all the ISRs and segmentation
    InitializeHeap();
    EnableCursor(0xE, 0xF);
    LoadDescTables();
    RegisterSysHandlers();

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
        Print("PC:>>");
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
            Print("Helloooo :)\n\n");
            continue;
        }

        // TO FIX: This not working (all just hangs)
        if(CompareString(command, "reboot"))
        {
            Print("This command doesn't work for now. It will be fixed soon.\n");
            continue;
        }

        if(CompareString(command, "cls"))
        {
            ClearScreen();
            continue;
        }

        if(CompareString(command, "system"))
        {
            Print("JuiceOS Kernel32 v" KERNEL_VERSION "\n\n");
            continue;
        }

        if(CompareString(command, "exit"))
        {
            return;
        }

        if(CompareString(command, "ascii"))
        {
            Print("Type any char.\n");

            uint8 key = ReadKey();
            Print("ASCII code of typed key is: 0x");
            PrintByteAsString(key);
            Print("\nIt displays as: ");

            PrintChar(key);
            Print("\n\n");
            continue;
        }

        if(CompareString(command, "memdump"))
        {
            OpenMemDumper();
            ClearScreen();
            continue;
        }

        if(CompareString(command, "help"))
        {
            Print("ASCII - Print hex representation of a typed char.\n");
            Print("CLS - Clear the console.\n");
            Print("EXIT - Quit from console to OS menu.\n");
            Print("HELP - Print this message.\n");
            Print("HELLO - Test command that say hello to you.\n");
            Print("MEMDUMP - Open Memory dumper.\n");
            Print("REBOOT - Reboot your PC.\n");
            Print("SCANTEST - Print scancode of every pressed key.\n");
            Print("SYSTEM - Print system information.\n\n");
            continue;
        }

        if(CompareString(command, "scantest"))
        {
            ps2_scancode(false);

            while(true)
            {
                uint8 scancode = ps2_scancode(false);

                Print("0x");
                PrintByteAsString(scancode);
                Print("\n");
                if(scancode == 0x81) break;
            }
            PrintChar('\n');
            continue;
        }

        if(CompareString(command, "\x1B"))
            return;
        
        Print("Unknown command.\n\n");
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
        Print("Juice OS v" KERNEL_VERSION " Menu");

        cursorX = 6;
        cursorY = 3;
        if(currentPosition == 0)
            PrintColor = SELECTED_COLOR;
        else 
            PrintColor = NON_SELECTED_COLOR;
        Print("Open console");

        cursorX = 6;
        cursorY = 4;
        if(currentPosition == 1)
            PrintColor = SELECTED_COLOR;
        else 
            PrintColor = NON_SELECTED_COLOR;
        Print("Memory dumper");

        cursorX = 6;
        cursorY = 5;
        if(currentPosition == 2)
            PrintColor = SELECTED_COLOR;
        else 
            PrintColor = NON_SELECTED_COLOR;
        Print("System logs");

        cursorX = 6;
        cursorY = 6;
        if(currentPosition == 3)
            PrintColor = SELECTED_COLOR;
        else 
            PrintColor = NON_SELECTED_COLOR;
        Print("Debug");

        // Getting input
        uint8 key = ps2_scancode(true);
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
                OpenMemDumper();
                ClearScreen();
            }
            if(currentPosition == 2)
            {
                OpenSysLogs();
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

void OpenMemDumper(void)
{
    uint8* memPtr = (uint8*) 0x0;
    uint8 asciiFlag = false;
    ClearScreen();

    while(true)
    {
        PrintColor = STANDART_INVERTED_SCREEN_COLOR;
        Print("                             Juice OS Memory Dumper                             ");

        PrintColor = STANDART_SCREEN_COLOR;
        cursorX = 0;
        cursorY = 2;

        Print("    Offset: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");

        cursorX = 0;
        cursorY = 4;

        for(int i=0; i < 256; i++)
        {
            if(asciiFlag)
            {
                Print("0x");
                PrintIntAsString((uint32) memPtr + i);
                Print(": ");

                for(int ii=0; ii < 16; ii++)
                {
                    PrintColor = 0x07;
                    print_char_noupdates(memPtr[i + ii]);
                    UpdateCursor();
                    PrintColor = STANDART_SCREEN_COLOR;
                    Print("  ");
                }
                i += 15;
                PrintChar('\n');
            } 
            else
            {
                Print("0x");
                PrintIntAsString((uint32) memPtr + i);
                Print(": ");

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
        Print("Dump: 0x");
        PrintIntAsString((uint32) memPtr);
        Print(" - 0x");
        PrintIntAsString((uint32) memPtr + 255);
        Print(" | ASCII Flag = ");
        if(asciiFlag)
            Print("ON ");
        else
            Print("OFF");
        print_string_noupdates("                                     ");

        cursorX = 0;
        cursorY = 0;
        UpdateCursor();

        uint8 key = ps2_scancode(true);
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

void OpenSysLogs(void)
{
    ClearScreen();
    Print((uint8*) &systemLogBuffer);
    ReadKey();
}

void OpenDebug(void)
{
    ClearScreen();

    while(true)
    {
        uint32 res;

        cursorX = 0;
        cursorY = 0;

        PrintColor = STANDART_INVERTED_SCREEN_COLOR;
        Print("                                    Debugger                                    ");
        PrintColor = STANDART_SCREEN_COLOR;

        // EBP ---------------------------------
        cursorX = 2;
        cursorY = 2;
        Print("EBP: 0x");

        __asm__("mov %%ebp, %%edx" : "=d" (res));
        PrintIntAsString(res);

        // ESP ---------------------------------
        cursorX = 22;
        cursorY = 2;
        Print("ESP: 0x");

        __asm__("mov %%esp, %%edx" : "=d" (res));
        PrintIntAsString(res);

        // CS ---------------------------------
        cursorX = 42;
        cursorY = 2;
        Print("CS: 0x");

        __asm__("mov %%cs, %%edx" : "=d" (res));
        PrintIntAsString(res);

        // DS ---------------------------------
        cursorX = 62;
        cursorY = 2;
        Print("DS: 0x");

        __asm__("mov %%ds, %%edx" : "=d" (res));
        PrintIntAsString(res);

        // ES ---------------------------------
        cursorX = 2;
        cursorY = 4;
        Print("ES: 0x");

        __asm__("mov %%es, %%edx" : "=d" (res));
        PrintIntAsString(res);

        // GS ---------------------------------
        cursorX = 22;
        cursorY = 4;
        Print("GS: 0x");

        __asm__("mov %%gs, %%edx" : "=d" (res));
        PrintIntAsString(res);

        // FS ---------------------------------
        cursorX = 42;
        cursorY = 4;
        Print("FS: 0x");

        __asm__("mov %%fs, %%edx" : "=d" (res));
        PrintIntAsString(res);

        // SS ---------------------------------
        cursorX = 62;
        cursorY = 4;
        Print("SS: 0x");

        __asm__("mov %%ss, %%edx" : "=d" (res));
        PrintIntAsString(res);

        // CR0 --------------------------------
        cursorX = 2;
        cursorY = 6;
        Print("CR0: 0x");

        __asm__("mov %%cr0, %%edx" : "=d" (res));
        PrintIntAsString(res);

        // CR2 --------------------------------
        cursorX = 22;
        cursorY = 6;
        Print("CR2: 0x");

        __asm__("mov %%cr2, %%edx" : "=d" (res));
        PrintIntAsString(res);

        // CR3 --------------------------------
        cursorX = 42;
        cursorY = 6;
        Print("CR3: 0x");

        __asm__("mov %%cr3, %%edx" : "=d" (res));
        PrintIntAsString(res);

        // CR4 --------------------------------
        cursorX = 62;
        cursorY = 6;
        Print("CR4: 0x");

        __asm__("mov %%cr4, %%edx" : "=d" (res));
        PrintIntAsString(res);

        cursorX = 2;
        cursorY = 8;
        Print("----------------------------------------------------------------------------");

        // System memory ----------------------
        cursorX = 2;
        cursorY = 10;
        Print("Memory allocated for the system: ");

        uint32 systemMemory = heapStartValue - 0x100000;
        if(systemMemory / 1048576 > 0)
        {
            PrintNum(systemMemory / 1048576);
            Print(" MB.");
        } 
        else if(systemMemory / 1024 > 0)
        {
            PrintNum(systemMemory / 1024);
            Print(" KB.");
        }
        else
        {
            PrintNum(systemMemory);
            Print(" B.");
        }

        // Heap -------------------------------
        cursorX = 2;
        cursorY = 12;
        Print("Heap allocated: ");

        uint32 allocatedHeap = currentHeapValue - heapStartValue;
        if(allocatedHeap / 1048576 > 0) 
        {
            PrintNum(allocatedHeap / 1048576);
            Print(" MB.");
        }
        else if(allocatedHeap / 1024 > 0)
        {
            PrintNum(allocatedHeap / 1024);
            Print(" KB.");
        }
        else
        {
            PrintNum(allocatedHeap);
            Print(" B.");
        }

        // Heap memory location ---------------
        cursorX = 2;
        cursorY = 14;
        Print("Heap located at: 0x");
        PrintIntAsString(heapStartValue);

        uint8 key = ps2_keyDown();
        if(key == 0x01) break;

        asm volatile("hlt");
    }
}