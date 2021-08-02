#include "inc/stdio.h"
#include "inc/ps2.h"
#include "inc/descriptor_tables.h"
#include "inc/kernel.h"
#include "inc/timer.h"

static uint8 systemLogBuffer[2048];

void kernel_init(void) {
    // Initialise all the ISRs and segmentation
    init_descriptor_tables();
    enable_cursor(0xE, 0xF);
    clear_screen();

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

        if(inputExitCode == 0xF0) {
            // If WinLeft pressed
            command = "exit";
        }

        if(str_len(command) == 0) {
            // If nothing was printed
            continue;
        }

        if(str_compare(command, "hello")) {
            print_string("Helloooo :)\n\n");
            continue;
        }

        if(str_compare(command, "reboot")) {
            reset_idt();
            __asm__("jmp 0xFFFF0");
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

        if(str_compare(command, "cpuid")) {
            print_string("CPUID: ");
            print_string(cpuid_get_id());
            print_string("\nCPU Model: ");
            print_string(cpuid_get_model());
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
            print_string("CPUID - Print CPUID information.\n");
            print_string("EXIT - Quit from console to OS menu.\n");
            print_string("HELP - Print this message.\n");
            print_string("HELLO - Test command that say hello to you.\n");
            print_string("MEMDUMP - Open Memory dumper.\n");
            print_string("PITTEST - Init PIT timer to test it.\n");
            print_string("REBOOT - Reboot your PC.\n");
            print_string("SCANTEST - Print scancode of every pressed key.\n");
            print_string("SYSTEM - Print system information.\n");
            print_string("TEST_INT10H - Test IDT. It calls 0x10 interrupt.\n\n");
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

        if(str_compare(command, "test_int10h")) {
            asm volatile("int $0x10");
            print_char('\n');
            continue;
        }

        if(str_compare(command, "pittest")) {
            init_timer(50);
            continue;
        }
        
        print_string("Your typed command is not found.\n\n");
    }
}

void openMenu(void) {
    clear_screen();
    const uint8 NON_SELECTED_COLOR = STANDART_SCREEN_COLOR;
    const uint8 SELECTED_COLOR = STANDART_INVERTED_SCREEN_COLOR;
    const uint8 ITEMS_AMOUNT = 4;         // Zero means - 1

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
        print_string("Information");

        cursorX = 6;
        cursorY = 6;
        if(currentPosition == 3)
            printColor = SELECTED_COLOR;
        else 
            printColor = NON_SELECTED_COLOR;
        print_string("Reboot PC");

        cursorX = 6;
        cursorY = 7;
        if(currentPosition == 4)
            printColor = SELECTED_COLOR;
        else 
            printColor = NON_SELECTED_COLOR;
        print_string("Show system logs");

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
                openInfo();
                clear_screen();
            }
            if(currentPosition == 3) {
                reset_idt();
                __asm__ ("jmp 0xFFFF0");
            }
            if(currentPosition == 4) {
                openSysLogs();
                clear_screen();
            }
        }
    }
}

void openInfo(void) {
    clear_screen();
    cursorX = 25;
    cursorY = 1;
    print_string("Juice OS - Kernel32 v" KERNEL_VERSION);

    cursorX = 34;
    cursorY = 2;
    print_string("by purepelmen");

    cursorX = 2;
    cursorY = 4;
    print_string("JuiceOS Kernel32 - it is my 32-bit operating system. For now there is only a");

    cursorX = 2;
    cursorY = 5;
    print_string("console with few commands and a menu. But I'm trying to develop this OS. I'm");

    cursorX = 2;
    cursorY = 6;
    print_string("finding some information on a different sites about ports description to ");

    cursorX = 2;
    cursorY = 7;
    print_string("improve OS features. I don't know what to say, so... maybe that's all that");

    cursorX = 2;
    cursorY = 8;
    print_string("I want to say. Good luck to you, man! Tq for testing my OS :)");

    cursorX = 2;
    cursorY = 9;
    print_string("Oh, and by the way, sorry for my bad English. I don't know it very well.");

    cursorX = 0;
    cursorY = 24;
    printColor = STANDART_INVERTED_SCREEN_COLOR;
    print_string_noupdates("                        Click any key to back to menu...                        ");
    ps2_readKey();
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
        for(int i=0; i < 512; i++) {
            if(asciiFlag) {
                printColor = 0x07;
                print_char_noupdates(memPtr[i]);
                update_cursor();
                printColor = STANDART_SCREEN_COLOR;
                print_string("  ");
            } else {
                printColor = 0x07;
                print_hexb(memPtr[i]);
                printColor = STANDART_SCREEN_COLOR;
                print_char(' ');
            }
        }

        printColor = STANDART_INVERTED_SCREEN_COLOR;
        cursorX = 0;
        cursorY = 24;
        print_string("Dump: 0x");
        print_hexdw((uint32) memPtr);
        print_string(" - 0x");
        print_hexdw((uint32) memPtr + 511);
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
            memPtr += 512;
        }
        if(key == 0x4B) {
            // Left arrow
            memPtr -= 512;
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