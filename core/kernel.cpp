#include "kernel.h"
#include "multiboot.h"

#include "gdt.h"
#include "idt.h"
#include "heap.h"
#include "paging.h"

#include "shell.h"
#include "console.h"

#include "drivers/screen.h"
#include "drivers/pit.h"
#include "drivers/ps2.h"
#include "drivers/pci.h"
#include "drivers/ide.h"

static void* multibootInfoStruct;
static multiboot_tag* firstMutlibootTag;

static const char* bootloaderName = "__UNDEFINED";

static char syslog_buffer[2048];

static void kernel_init();
static void kernel_analyze_multiboot_struct();

extern "C" void kernel_main(void* multibootDataFromBootloader)
{
    multibootInfoStruct = multibootDataFromBootloader;

    kernel_analyze_multiboot_struct();
    kernel_init();

    kshell::open_menu();
}

void kernel_init()
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
    kide::init();
    
    kernel_print_log("Kernel initialization completed.\n");
    kconsole::printf("Booted by: %s.\n", bootloaderName);

    kconsole::printf("Press any key to continue...");
    kconsole::read_string();
}

void kernel_analyze_multiboot_struct()
{
    firstMutlibootTag = (multiboot_tag*) ((uint8*)multibootInfoStruct + 8);

    multiboot_tag* blNameTag = multiboot_find_tag(firstMutlibootTag, multiboot_tagtype::BOOTLOADER_NAME);
    if (!blNameTag->is_end())
    {
        bootloaderName = (char*)&blNameTag->data;
    }
    else
    {
        kernel_print_log("multiboot: Failed to get the bootloader name. The tag is not present.\n");
    }
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
