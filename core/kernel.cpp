#include "kernel.h"

#include "gdt.h"
#include "idt.h"
#include "heap.h"
#include "paging.h"

#include "shell.h"
#include "console.h"

#include "drivers/screen.h"
#include "drivers/ahci.h"
#include "drivers/pit.h"
#include "drivers/ps2.h"
#include "drivers/pci.h"
#include "drivers/ide.h"

static char syslog_buffer[2048];

static void init_kernel();

extern "C" void kernel_main()
{
    init_kernel();
    kshell::open_menu();
}

void init_kernel()
{
    kscreen::clear();
    kscreen::enable_cursor(0xE, 0xF);

    kheap::init();

    kgdt::gdt_init();
    kidt::idt_init();
    kpaging::paging_init();

    ktimer::init();
    kps2::init();
    
    kpci::init();
    kahci::init();
    kide::init();
    
    kernel_print_log("Kernel initialization completed.\n");

    kconsole::printf("Press any key to continue...");
    kconsole::read_string();
}

void kernel_print_log(string str)
{
    string buffer = syslog_buffer;
    
    if(buffer.length() + str.length() > sizeof(syslog_buffer) - 2) return;
    buffer.concat(str);
}

string kernel_read_logs()
{
    return string(syslog_buffer);
}
