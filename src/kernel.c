#include "includes/stdio.h"
#include "includes/types.h"
#include "includes/ps2.h"

extern char* get_cpuid_info(void);
extern char* get_cpu_model_info(void);

void kernel_main(void) {
    clear_screen();

    while(1) {
        print_string("PC:>>");

        unsigned char* command = get_input();
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
            unsigned char key = ps2_waitKey();
            print_string("ASCII code of typed key is: 0x");
            print_hexb(key);
            print_string("\nIt displays as: ");
            print_char(key);
            print_string("\n\n");
            continue;
        }

        if(compare_string(command, "cpuid")) {
            print_string("CPUID: ");
            print_string(get_cpuid_info());
            print_string("\nCPU Model: ");
            print_string(get_cpu_model_info());
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
            print_string("Please type here device you want to test: ");
            unsigned char* subCmd = get_input();
            
            if(compare_string(subCmd, "ps2")) {
                port_byte_out(0x60, 0xEE);
                unsigned char response = port_byte_in(0x60);

                if(response == 0xEE) {
                    print_string("\nPS/2 keyboard responded to echo-command. PS/2 Keyboard passed the test.\n\n");
                    continue;
                }

                print_string("\nPS/2 keyboard not responded properly to echo-command. PS/2 Keyboard failed the test.\n\n");
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