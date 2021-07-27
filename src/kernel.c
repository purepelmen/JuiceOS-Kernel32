#include "includes/stdio.h"

#define KERNEL_VERSION "1.0.1"

extern uint8_t* cpuid_get_id(void);
extern uint8_t* cpuid_get_model(void);

void openMenu(void);
void openInfo(void);

void kernel_main(void) {
    openMenu();

    // Halting CPU
    clear_screen();
    cursorY = 24;
    cursorX = 13;
    print_string("CPU is halted! Turn off PC to exit from this state.");
    return;
}

void console(void) {
    clear_screen();

    while(1) {
        print_string("PC:>>");

        uint8_t* command = get_input();
        str_lower(command, command);
        print_char(0xA);

        if(inputExitCode == 0xF0) {
            // If WinLeft pressed
            command = "exit";
        }

        if(strlen(command) == 0) {
            // If nothing was printed
            continue;
        }

        if(compare_string(command, "hello")) {
            print_string("Helloooo :)\n\n");
            continue;
        }

        if(compare_string(command, "reboot")) {
            __asm__("jmp 0xFFFF0");
            continue;
        }

        if(compare_string(command, "cls")) {
            clear_screen();
            continue;
        }

        if(compare_string(command, "system")) {
            print_string("JuiceOS Kernel32 v" KERNEL_VERSION "\n\n");
            continue;
        }

        if(compare_string(command, "exit")) {
            return;
        }

        if(compare_string(command, "ascii")) {
            print_string("Type any char.\n");

            uint8_t key = ps2_waitKey();
            print_string("ASCII code of typed key is: 0x");
            print_hexb(key);
            print_string("\nIt displays as: ");

            print_char(key);
            print_string("\n\n");
            continue;
        }

        if(compare_string(command, "cpuid")) {
            print_string("CPUID: ");
            print_string(cpuid_get_id());
            print_string("\nCPU Model: ");
            print_string(cpuid_get_model());
            print_string("\n\n");
            continue;
        }

        if(compare_string(command, "help")) {
            print_string("ASCII - Print hex representation of a typed char.\n");
            print_string("CLS - Clear the console.\n");
            print_string("CPUID - Print CPUID information.\n");
            print_string("EXIT - Quit from console to OS menu.\n");
            print_string("HELP - Print this message.\n");
            print_string("HELLO - Test command that say hello to you.\n");
            print_string("REBOOT - Reboot your PC.\n");
            print_string("SCANCODE_TEST - Print scancode in hex of every pressed key.\n");
            print_string("SYSTEM - Print system information.\n");
            print_string("TEST - Test a hardware (for instance PS/2).\n\n");
            continue;
        }

        if(compare_string(command, "scancode_test")) {
            ps2_keyboard_getKey();

            while (1) {
                uint8_t scancode = ps2_keyboard_getKey();
                print_string("0x");
                print_hexb(scancode);
                print_string("\n");
            }
            
            continue;
        }

        if(compare_string(command, "test")) {
            print_string("Type device: ");
            uint8_t* subCmd = get_input();
            
            if(compare_string(subCmd, "ps2")) {
                port_byte_out(0x60, 0xEE);
                uint8_t response = port_byte_in(0x60);

                if(response == 0xEE) {
                    print_string("\nPS/2 keyboard - OK.\n\n");
                    continue;
                }

                print_string("\nPS/2 keyboard - FAIL.\n\n");
                continue;
            }

            if(compare_string(subCmd, "list")) {
                print_string("\nAvailable devices to test: ps2 (PS/2 Keyboard).\n\n");
                continue;
            }

            print_string("\nTyped device is not found! Type \"test\" and then \"list\" to get all devices you could test.\n\n");

            continue;
        }
        
        print_string("Your typed command is not found.\n\n");
    }
}

void openMenu(void) {
    clear_screen();
    const uint8_t NON_SELECTED_COLOR = STANDART_SCREEN_COLOR;
    const uint8_t SELECTED_COLOR = STANDART_INVERTED_SCREEN_COLOR;
    const uint8_t ITEMS_AMOUNT = 3;         // Zero means - 1

    uint8_t currentPosition = 0;
    while(1) {
        printColor = NON_SELECTED_COLOR;

        cursorX = 30;
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
        print_string("Information");

        cursorX = 6;
        cursorY = 5;
        if(currentPosition == 2)
            printColor = SELECTED_COLOR;
        else 
            printColor = NON_SELECTED_COLOR;
        print_string("Halt CPU");

        cursorX = 6;
        cursorY = 6;
        if(currentPosition == 3)
            printColor = SELECTED_COLOR;
        else 
            printColor = NON_SELECTED_COLOR;
        print_string("Reboot PC");

        // Getting input
        uint8_t key = ps2_keyboard_getKey();
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
                openInfo();
                clear_screen();
            }
            if(currentPosition == 2) {
                return;
            }
            if(currentPosition == 3) {
                __asm__ ("jmp 0xFFFF0");
            }
        }
    }
}

void openInfo(void) {
    clear_screen();
    cursorX = 30;
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
    printColor = STANDART_SCREEN_COLOR;
    ps2_waitKey();
}