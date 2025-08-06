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
static int syslog_length = 0;

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
    kconsole::clear();
    kscreen::enable_hwcursor(0xE, 0xF);

    kheap::init();

    kgdt::gdt_init();
    kidt::idt_init();
    kpaging::paging_init();

    ktimer::init();
    kps2::init();
    
    kpci::init();
    kide::init();
    
    kernel_log("Kernel initialization completed.\n");

    for (int i = 0; i < 40; i++)
    {
        kernel_log("Simple text: %d.\n", i);
    }

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
        kernel_log("Booted by: %s.\n", bootloaderName);
    }
    else
    {
        kernel_log("multiboot: Failed to get the bootloader name. The tag is not present.\n");
    }
}

void kernel_log(string str, ...)
{
    va_list args;

    va_start(args, str);
    vsprintf([](void* context, const char* portionPtr, int length) 
    {
        if (syslog_length + length > sizeof(syslog_buffer) - 1)
            return;

        mem_copy(portionPtr, &syslog_buffer[syslog_length], length);
        syslog_length += length;
    }, nullptr, str.ptr(), args);
    va_end(args);
    
    syslog_buffer[syslog_length] = 0x0;
}

bool kernel_render_logs(int pageIndex)
{
    const size_t MAX_SCREENSPACE = kscreen::width() * (kscreen::height() - 1);
    
    int currentOffset = 0;
    for (int i = 0; i < pageIndex; i++)
    {
        currentOffset += kconsole::calc_fit_substring(syslog_buffer + currentOffset, MAX_SCREENSPACE);
    }

    int chunkLength = kconsole::calc_fit_substring(syslog_buffer + currentOffset, MAX_SCREENSPACE);
    kconsole::print(syslog_buffer + currentOffset, chunkLength);

    return currentOffset + chunkLength < syslog_length;
}
