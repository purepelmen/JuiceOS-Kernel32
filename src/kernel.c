#include "includes/stdio.h"

extern uint8_t* cpuid_get_id(void);
extern uint8_t* cpuid_get_model(void);

void kernel_main(void) {
    clear_screen();

    while(1) {
        print_string("PC:>>");

        uint8_t* command = get_input();
        str_lower(command, command);
        print_char(0xA);

        if(strlen(command) == 0) {
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
            print_string("JuiceOS Kernel32 v1.0\n\n");
            continue;
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
            print_string("HELP - Print this message.\n");
            print_string("HELLO - Test command that say hello to you.\n");
            print_string("REBOOT - Reboot your PC.\n");
            print_string("SYSTEM - Print system information.\n");
            print_string("TEST - Test a hardware (for instance PS/2).\n\n");
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