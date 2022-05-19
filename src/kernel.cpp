#include "drivers/screen.hpp"
#include "drivers/ps2.hpp"

#include "kernel.hpp"
#include "heap.hpp"
#include "gdt.hpp"

// Buffer of the system log
static char systemLogBuffer[2048];

ScreenDriver screen;
Ps2 ps2;

void init_kernel()
{
    kgdt::gdt_init();
    kernel_print_log("GDT initialized.\n");

    screen.initialize();
    ps2.initialize();
    kernel_print_log("Drivers initialized.\n");

    init_heap();

    screen.enable_cursor(0xE, 0xF);
    screen.clear();

    kernel_print_log("Kernel initialization completed.\n");
}

extern "C" void kernel_main()
{
    init_kernel();
    open_menu();
}

void open_console(void)
{
    screen.clear();

    while(true)
    {
        screen.printColor = STANDART_CONSOLE_PREFIX_COLOR;
        screen << "PC:>>";
        screen.printColor = STANDART_SCREEN_COLOR;

        string command = read_string();
        command.to_lower(command);
        screen << 0xA;

        if(command.length() == 0)
        {
            // If nothing was printed
            continue;
        }

        if(command == "hello")
        {
            screen << "Helloooo :)\n\n";
            continue;
        }

        // Awaiting IDT re-implementation
        // TO FIX: This not working (all just hangs) 
        if(command == "reboot")
        {
            screen << "This command doesn't work for now. It will be fixed soon.\n\n";
            continue;
        }

        if(command == "cls")
        {
            screen.clear();
            continue;
        }

        if(command == "system")
        {
            screen << "JuiceOS Kernel32 v" KERNEL_VERSION "\n\n";
            continue;
        }

        if(command == "exit")
        {
            return;
        }

        if(command == "ascii")
        {
            screen << "Type any char.\n";

            uint8 key = ps2.read_ascii();
            screen << "ASCII code of typed key is: 0x";
            print_hex_8bit(key);
            screen << "\nIt displays as: ";

            screen << key;
            screen << "\n\n";
            continue;
        }

        if(command == "memdump")
        {
            open_memdumper();
            screen.clear();
            continue;
        }

        if(command == "help")
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

        if(command == "scantest")
        {
            ps2.get_scancode(false);

            while(true)
            {
                uint8 scancode = ps2.get_scancode(false);

                screen << "0x";
                print_hex_8bit(scancode);
                screen << "\n";
                if(scancode == 0x81) break;
            }
            
            screen << '\n';
            continue;
        }

        if(command == "\x1B")
            return;
        
        screen << "Unknown command.\n\n";
    }
}

void open_menu(void)
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
        uint8 key = ps2.get_scancode(true);
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
                open_console();
                screen.clear();
            }
            if(currentPosition == 1)
            {
                open_memdumper();
                screen.clear();
            }
            if(currentPosition == 2)
            {
                open_syslogs();
                screen.clear();
            }
            if(currentPosition == 3)
            {
                open_debugger();
                screen.clear();
            }
        }
    }
}

void open_memdumper(void)
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
                print_hex_32bit((uint32) memPtr + i);
                screen << ": ";

                for(int ii = 0; ii < 16; ii++)
                {
                    screen.printColor = 0x07;
                    screen.print_char_noupd(memPtr[i + ii]);
                    screen.update_cursor();
                    screen.printColor = STANDART_SCREEN_COLOR;
                    screen << "  ";
                }

                i += 15;
                screen << '\n';
            } 
            else
            {
                screen <<"0x";
                print_hex_32bit((uint32) memPtr + i);
                screen << ": ";

                for(int ii = 0; ii < 16; ii++)
                {
                    screen.printColor = 0x07;
                    print_hex_8bit(memPtr[i + ii]);
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
        print_hex_32bit((uint32) memPtr);
        screen << " - 0x";
        print_hex_32bit((uint32) memPtr + 255);
        screen << " | ASCII Flag = ";
        if(asciiFlag)
            screen << "ON ";
        else
            screen << "OFF";
        screen.print_string_noupd("                                     ");

        screen.cursorX = 0;
        screen.cursorY = 0;
        screen.update_cursor();

        uint8 key = ps2.get_scancode(true);
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

void kernel_print_log(string str)
{
    if(string(systemLogBuffer).length() + str.length() > 2046) return;
    string(systemLogBuffer).concat(str);
}

void open_syslogs(void)
{
    screen.clear();
    screen << string(systemLogBuffer);
    ps2.read_ascii();
}

void open_debugger(void)
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
    print_hex_32bit(res);

    // ESP ---------------------------------
    screen.cursorX = 22;
    screen.cursorY = 2;
    screen << "ESP: 0x";

    __asm__("mov %%esp, %%edx" : "=d" (res));
    print_hex_32bit(res);

    // CS ---------------------------------
    screen.cursorX = 42;
    screen.cursorY = 2;
    screen << "CS: 0x";

    __asm__("mov %%cs, %%edx" : "=d" (res));
    print_hex_32bit(res);

    // DS ---------------------------------
    screen.cursorX = 62;
    screen.cursorY = 2;
    screen << "DS: 0x";

    __asm__("mov %%ds, %%edx" : "=d" (res));
    print_hex_32bit(res);

    // ES ---------------------------------
    screen.cursorX = 2;
    screen.cursorY = 4;
    screen << "ES: 0x";

    __asm__("mov %%es, %%edx" : "=d" (res));
    print_hex_32bit(res);

    // GS ---------------------------------
    screen.cursorX = 22;
    screen.cursorY = 4;
    screen << "GS: 0x";

    __asm__("mov %%gs, %%edx" : "=d" (res));
    print_hex_32bit(res);

    // FS ---------------------------------
    screen.cursorX = 42;
    screen.cursorY = 4;
    screen << "FS: 0x";

    __asm__("mov %%fs, %%edx" : "=d" (res));
    print_hex_32bit(res);

    // SS ---------------------------------
    screen.cursorX = 62;
    screen.cursorY = 4;
    screen << "SS: 0x";

    __asm__("mov %%ss, %%edx" : "=d" (res));
    print_hex_32bit(res);

    // CR0 --------------------------------
    screen.cursorX = 2;
    screen.cursorY = 6;
    screen << "CR0: 0x";

    __asm__("mov %%cr0, %%edx" : "=d" (res));
    print_hex_32bit(res);

    // CR2 --------------------------------
    screen.cursorX = 22;
    screen.cursorY = 6;
    screen << "CR2: 0x";

    __asm__("mov %%cr2, %%edx" : "=d" (res));
    print_hex_32bit(res);

    // CR3 --------------------------------
    screen.cursorX = 42;
    screen.cursorY = 6;
    screen << "CR3: 0x";

    __asm__("mov %%cr3, %%edx" : "=d" (res));
    print_hex_32bit(res);

    // CR4 --------------------------------
    screen.cursorX = 62;
    screen.cursorY = 6;
    screen << "CR4: 0x";

    __asm__("mov %%cr4, %%edx" : "=d" (res));
    print_hex_32bit(res);

    screen.cursorX = 2;
    screen.cursorY = 8;
    screen << "----------------------------------------------------------------------------";

    // System memory ----------------------
    screen.cursorX = 2;
    screen.cursorY = 10;
    screen << "Memory allocated for the system: ";

    uint32 systemMemory = head_start_value - 0x100000;
    if(systemMemory / 1048576 > 0)
    {
        print_number(systemMemory / 1048576);
        screen << " MB.";
    } 
    else if(systemMemory / 1024 > 0)
    {
        print_number(systemMemory / 1024);
        screen << " KB.";
    }
    else
    {
        print_number(systemMemory);
        screen << " B.";
    }

    // Heap -------------------------------
    screen.cursorX = 2;
    screen.cursorY = 12;
    screen << "Heap allocated: ";

    uint32 allocatedHeap = current_heap_value - head_start_value;
    if(allocatedHeap / 1048576 > 0) 
    {
        print_number(allocatedHeap / 1048576);
        screen << " MB.";
    }
    else if(allocatedHeap / 1024 > 0)
    {
        print_number(allocatedHeap / 1024);
        screen << " KB.";
    }
    else
    {
        print_number(allocatedHeap);
        screen << " B.";
    }

    // Heap memory location ---------------
    screen.cursorX = 2;
    screen.cursorY = 14;
    screen << "Heap located at: 0x";
    print_hex_32bit(head_start_value);

    while(true)
    {
        uint8 key = ps2.get_current_key();
        if(key == 0x01) return;
    }
}
