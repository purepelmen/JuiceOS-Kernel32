#include "stdio.h"
#include "drivers/screen.h"
#include "drivers/ps2.h"
#include "heap.h"

#include "isr/gdt_idt.h"
#include "kernel.h"
#include "system.h"

// Buffer of the system log
static uint8 systemLogBuffer[2048];

void kernel_init(void) {
    // Initialise all the ISRs and segmentation
    init_heap();
    clear_screen();
    enable_cursor(0xE, 0xF);
    loadDescriptorTables();
    registerSystemHandlers();
    
    print_log("Kernel initialization completed.\n");
}

void kernel_main(void) {
    kernel_init();
    openMenu();
}

void console(void) {
    clear_screen();

    while(1) {
        printColor = STANDART_CONSOLE_PREFIX_COLOR;
        print_string("PC:>>");
        printColor = STANDART_SCREEN_COLOR;

        uint8* command = get_input();
        str_lower(command, command);
        print_char(0xA);

        if(str_len(command) == 0) {
            // If nothing was printed
            continue;
        }

        if(str_compare(command, "hello")) {
            print_string("Helloooo :)\n\n");
            continue;
        }

        // TO FIX: This not working (all just hangs)
        if(str_compare(command, "reboot")) {
            print_string("This does not work for now.");
            continue;
        }

        if(str_compare(command, "cls")) {
            clear_screen();
            continue;
        }

        if(str_compare(command, "system")) {
            print_string("JuiceOS Kernel32 v" KERNEL_VERSION "\n\n");
            continue;
        }

        if(str_compare(command, "exit")) {
            return;
        }

        if(str_compare(command, "ascii")) {
            print_string("Type any char.\n");

            uint8 key = ps2_readKey();
            print_string("ASCII code of typed key is: 0x");
            print_hexb(key);
            print_string("\nIt displays as: ");

            print_char(key);
            print_string("\n\n");
            continue;
        }

        if(str_compare(command, "memdump")) {
            openMemoryDumper();
            clear_screen();
            continue;
        }

        if(str_compare(command, "help")) {
            print_string("ASCII - Print hex representation of a typed char.\n");
            print_string("CLS - Clear the console.\n");
            print_string("EXIT - Quit from console to OS menu.\n");
            print_string("HELP - Print this message.\n");
            print_string("HELLO - Test command that say hello to you.\n");
            print_string("MEMDUMP - Open Memory dumper.\n");
            print_string("REBOOT - Reboot your PC.\n");
            print_string("SCANTEST - Print scancode of every pressed key.\n");
            print_string("SYSTEM - Print system information.\n\n");
            continue;
        }

        if(str_compare(command, "scantest")) {
            ps2_scancode(false);

            while (1) {
                uint8 scancode = ps2_scancode(false);

                print_string("0x");
                print_hexb(scancode);
                print_string("\n");
                if(scancode == 0x81) break;
            }
            print_char('\n');
            continue;
        }

        if(str_compare(command, "\x1B")) {
            return;
        }
        
        print_string("Unknown command.\n\n");
    }
}

void openMenu(void) {
    clear_screen();
    const uint8 NON_SELECTED_COLOR = STANDART_SCREEN_COLOR;
    const uint8 SELECTED_COLOR = STANDART_INVERTED_SCREEN_COLOR;
    const uint8 ITEMS_AMOUNT = 3;         // Zero means - 1

    uint8 currentPosition = 0;
    while(1) {
        printColor = NON_SELECTED_COLOR;

        cursorX = 25;
        cursorY = 1;
        print_string("Juice OS v" KERNEL_VERSION " Menu");

        cursorX = 6;
        cursorY = 3;
        if(currentPosition == 0)
            printColor = SELECTED_COLOR;
        else 
            printColor = NON_SELECTED_COLOR;
        print_string("Open console");

        cursorX = 6;
        cursorY = 4;
        if(currentPosition == 1)
            printColor = SELECTED_COLOR;
        else 
            printColor = NON_SELECTED_COLOR;
        print_string("Memory dumper");

        cursorX = 6;
        cursorY = 5;
        if(currentPosition == 2)
            printColor = SELECTED_COLOR;
        else 
            printColor = NON_SELECTED_COLOR;
        print_string("System logs");

        cursorX = 6;
        cursorY = 6;
        if(currentPosition == 3)
            printColor = SELECTED_COLOR;
        else 
            printColor = NON_SELECTED_COLOR;
        print_string("Debug");

        // Getting input
        uint8 key = ps2_scancode(true);
        if(key == 0x48) {
            if(currentPosition == 0) currentPosition = ITEMS_AMOUNT;
            else currentPosition--;
        }
        if(key == 0x50) {
            if(currentPosition == ITEMS_AMOUNT) currentPosition = 0;
            else currentPosition++;
        }
        if(key == 0x1C) {
            printColor = NON_SELECTED_COLOR;
            if(currentPosition == 0) {
                console();
                clear_screen();
            }
            if(currentPosition == 1) {
                openMemoryDumper();
                clear_screen();
            }
            if(currentPosition == 2) {
                openSysLogs();
                clear_screen();
            }
            if(currentPosition == 3) {
                openDebug();
                clear_screen();
            }
        }
    }
}

void openMemoryDumper(void) {
    uint8* memPtr = (uint8*) 0x0;
    uint8 asciiFlag = false;
    clear_screen();

    while(1) {
        printColor = STANDART_INVERTED_SCREEN_COLOR;
        print_string("                             Juice OS Memory Dumper                             ");

        printColor = STANDART_SCREEN_COLOR;
        cursorX = 0;
        cursorY = 2;

        print_string("    Offset: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");

        cursorX = 0;
        cursorY = 4;

        for(int i=0; i < 256; i++) {
            if(asciiFlag) {
                print_string("0x");
                print_hexdw((uint32) memPtr + i);
                print_string(": ");

                for(int ii=0; ii < 16; ii++) {
                    printColor = 0x07;
                    print_char_noupdates(memPtr[i + ii]);
                    update_cursor();
                    printColor = STANDART_SCREEN_COLOR;
                    print_string("  ");
                }
                i += 15;
                print_char('\n');
            } else {
                print_string("0x");
                print_hexdw((uint32) memPtr + i);
                print_string(": ");

                for(int ii=0; ii < 16; ii++) {
                    printColor = 0x07;
                    print_hexb(memPtr[i + ii]);
                    printColor = STANDART_SCREEN_COLOR;
                    print_char(' ');
                }
                i += 15;
                print_char('\n');
            }
        }

        printColor = STANDART_INVERTED_SCREEN_COLOR;
        cursorX = 0;
        cursorY = 24;
        print_string("Dump: 0x");
        print_hexdw((uint32) memPtr);
        print_string(" - 0x");
        print_hexdw((uint32) memPtr + 255);
        print_string(" | ASCII Flag = ");
        if(asciiFlag)
            print_string("ON ");
        else
            print_string("OFF");
        print_string_noupdates("                                     ");

        cursorX = 0;
        cursorY = 0;
        update_cursor();

        uint8 key = ps2_scancode(true);
        if(key == 0x01) {
            break;
        }
        if(key == 0x4D) {
            // Right arrow
            memPtr += 0x100;
        }
        if(key == 0x4B) {
            // Left arrow
            memPtr -= 0x100;
        }
        if(key == 0x50) {
            // Down arrow
            memPtr += 0x1000;
        }
        if(key == 0x48) {
            // Up arrow
            memPtr -= 0x1000;
        }
        if(key == 0x2A) {
            asciiFlag = !asciiFlag;
        }
    }
}

void print_log(uint8* str) {
    if(str_len((uint8*) &systemLogBuffer) + str_len(str) > 2046) return;
    str_concat((uint8*) &systemLogBuffer, str);
}

void openSysLogs(void) {
    clear_screen();
    print_string((uint8*) &systemLogBuffer);
    ps2_readKey();
}

void openDebug(void) {
    clear_screen();

    while(1) {
        uint32 res;

        cursorX = 0;
        cursorY = 0;

        printColor = STANDART_INVERTED_SCREEN_COLOR;
        print_string("                                    Debugger                                    ");
        printColor = STANDART_SCREEN_COLOR;

        // EBP ---------------------------------
        cursorX = 2;
        cursorY = 2;
        print_string("EBP: 0x");

        __asm__("mov %%ebp, %%edx" : "=d" (res));
        print_hexdw(res);

        // ESP ---------------------------------
        cursorX = 22;
        cursorY = 2;
        print_string("ESP: 0x");

        __asm__("mov %%esp, %%edx" : "=d" (res));
        print_hexdw(res);

        // CS ---------------------------------
        cursorX = 42;
        cursorY = 2;
        print_string("CS: 0x");

        __asm__("mov %%cs, %%edx" : "=d" (res));
        print_hexdw(res);

        // DS ---------------------------------
        cursorX = 62;
        cursorY = 2;
        print_string("DS: 0x");

        __asm__("mov %%ds, %%edx" : "=d" (res));
        print_hexdw(res);

        // ES ---------------------------------
        cursorX = 2;
        cursorY = 4;
        print_string("ES: 0x");

        __asm__("mov %%es, %%edx" : "=d" (res));
        print_hexdw(res);

        // GS ---------------------------------
        cursorX = 22;
        cursorY = 4;
        print_string("GS: 0x");

        __asm__("mov %%gs, %%edx" : "=d" (res));
        print_hexdw(res);

        // FS ---------------------------------
        cursorX = 42;
        cursorY = 4;
        print_string("FS: 0x");

        __asm__("mov %%fs, %%edx" : "=d" (res));
        print_hexdw(res);

        // SS ---------------------------------
        cursorX = 62;
        cursorY = 4;
        print_string("SS: 0x");

        __asm__("mov %%ss, %%edx" : "=d" (res));
        print_hexdw(res);

        // CR0 --------------------------------
        cursorX = 2;
        cursorY = 6;
        print_string("CR0: 0x");

        __asm__("mov %%cr0, %%edx" : "=d" (res));
        print_hexdw(res);

        // CR2 --------------------------------
        cursorX = 22;
        cursorY = 6;
        print_string("CR2: 0x");

        __asm__("mov %%cr2, %%edx" : "=d" (res));
        print_hexdw(res);

        // CR3 --------------------------------
        cursorX = 42;
        cursorY = 6;
        print_string("CR3: 0x");

        __asm__("mov %%cr3, %%edx" : "=d" (res));
        print_hexdw(res);

        // CR4 --------------------------------
        cursorX = 62;
        cursorY = 6;
        print_string("CR4: 0x");

        __asm__("mov %%cr4, %%edx" : "=d" (res));
        print_hexdw(res);

        cursorX = 2;
        cursorY = 8;
        print_string("----------------------------------------------------------------------------");

        // Heap -------------------------------
        cursorX = 2;
        cursorY = 10;
        print_string("Heap allocated: ");

        uint32 allocatedHeap = currentHeapValue - heapStartValue;
        if(allocatedHeap / 1048576 > 0) {
            print_number(allocatedHeap / 1048576);
            print_string(" MB.");
        } else if(allocatedHeap / 1024 > 0) {
            print_number(allocatedHeap / 1024);
            print_string(" KB.");
        } else {
            print_number(allocatedHeap);
            print_string(" B.");
        }

        uint8 key = ps2_keyDown();
        if(key == 0x01) {
            break;
        }

        asm volatile("hlt");
    }
}