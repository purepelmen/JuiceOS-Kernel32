#include "kernel.h"
#include "multiboot.h"

#include "gdt.h"
#include "idt.h"
#include "heap.h"
#include "paging.h"

#include "math.h"
#include "shell.h"
#include "console.h"

#include "drivers/screen.h"
#include "drivers/pit.h"
#include "drivers/ps2.h"
#include "drivers/pci.h"
#include "drivers/ide.h"
#include "drivers/storage.h"

static void* multibootInfoStruct;
static multiboot_tag* firstMutlibootTag;

static const char* bootloaderName = "__UNDEFINED";

const int SYSLOG_LENGTH = 2048;

static char syslog_buffer[SYSLOG_LENGTH];
static int syslog_writePtr = 0;
static bool syslog_isWrapped = false;

static bool syslog_printImmediately = true;

static void kernel_init();
static void kernel_analyze_multiboot_struct();

extern "C" void kernel_main(void* multibootDataFromBootloader)
{
    multibootInfoStruct = multibootDataFromBootloader;

    kernel_init();
    kshell::open_menu();
}

void kernel_init()
{
    kconsole::clear();
    kscreen::enable_hwcursor(0xE, 0xF);

    kernel_analyze_multiboot_struct();

    kheap::init();

    kgdt::gdt_init();
    kidt::idt_init();
    kpaging::paging_init();

    ktimer::init();
    kps2::init();
    
    kpci::init();
    kide::init();

    // This should be the last step as this is the only place where automount is perfomed AFTER disk drivers registered all found drives.
    kstorage::init();
    
    kernel_log("Kernel initialization completed.\n");
    syslog_printImmediately = false;

    kconsole::printf("Press any key to continue...");
    kconsole::read_string();
}

void kernel_analyze_multiboot_struct()
{
    firstMutlibootTag = (multiboot_tag*) ((uint8*)multibootInfoStruct + 8);

    multiboot_tag* blNameTag = multiboot_find_tag(firstMutlibootTag, multiboot_tagtype::BOOTLOADER_NAME);
    if (!blNameTag->is_end())
    {
        bootloaderName = (char*)&blNameTag->data.ptr;
        kernel_log("Booted by: %s.\n", bootloaderName);
    }
    else
    {
        kernel_log("multiboot: Failed to get the bootloader name. The tag is not present.\n");
    }

    multiboot_tag* basicMemInfo = multiboot_find_tag(firstMutlibootTag, multiboot_tagtype::BASIC_MEMORY_INFO);
    if (!basicMemInfo->is_end())
    {
        uint32 memory = basicMemInfo->data.basicMemInfo.memUpper + basicMemInfo->data.basicMemInfo.memLower + 1024;
        kernel_log("Available RAM: %dMB.\n", memory / 1024 );
    }

    multiboot_tag* mmapTag = multiboot_find_tag(firstMutlibootTag, multiboot_tagtype::MEMORY_MAP);
    if (!mmapTag->is_end())
    {
        kernel_log("Memory map table --------------------\n");

        size_t entrySize = mmapTag->data.memoryMap.entrySize;
        size_t i = 0;
        while (i < mmapTag->size)
        {
            auto& entry = mmapTag->data.memoryMap.entries[i / entrySize];
            
            auto base = entry.baseAddr;
            auto end = entry.baseAddr + entry.length - 1ULL;
            
            if (base > 0xFFFFFFFF || end > 0xFFFFFFFF)
                kernel_log("0x%llx-0x%llx -> ", base, end);
            else
                kernel_log("%p-%p -> ", (uint32)base, (uint32)end);

            const char* type = mb2_mmap_type_to_str(entry.type);
            if (type != nullptr)
                kernel_log(type);
            else
                kernel_log("UNKNOWN/RESERVED (%d)", entry.type);

            kernel_log("\n");
            i += entrySize;
        }
    }
}

void kernel_log(string str, ...)
{
    va_list args, args2;
    va_start(args, str);
    va_copy(args2, args);

    vsprintf([](void* context, const char* portionPtr, int length) 
    {
        // Clamp, as the code below can't handle such very large log portions.
        if (length > SYSLOG_LENGTH)
        {
            portionPtr += length - SYSLOG_LENGTH;
            length = SYSLOG_LENGTH;
        }

        int excess = max(0, syslog_writePtr + length - SYSLOG_LENGTH);
        int firstPortionLength = length - excess;

        mem_copy(portionPtr, &syslog_buffer[syslog_writePtr], firstPortionLength);
        syslog_writePtr += firstPortionLength;
        
        if (excess > 0)
        {
            mem_copy(portionPtr + firstPortionLength, &syslog_buffer[0], excess);
            syslog_writePtr = excess;
            
            syslog_isWrapped = true;
        }
    }, nullptr, str.ptr(), args);
    va_end(args);

    if (syslog_printImmediately)
        kconsole::vprintf(str, args);
        
    va_end(args2);
}

static char flatBuffer[SYSLOG_LENGTH + 1];
static const char* flatten_logs()
{
    int destIndex = 0;
    if (syslog_isWrapped)
    {
        int oldPortionLength = SYSLOG_LENGTH - syslog_writePtr;
        mem_copy(&syslog_buffer[syslog_writePtr], &flatBuffer[0], oldPortionLength);
        
        destIndex += oldPortionLength;
    }

    mem_copy(&syslog_buffer[0], &flatBuffer[destIndex], syslog_writePtr);
    destIndex += syslog_writePtr;

    flatBuffer[destIndex] = 0x0;
    return flatBuffer;
}

bool kernel_render_logs(int pageIndex)
{
    const size_t MAX_SCREENSPACE = kscreen::width() * (kscreen::height() - 1);
    
    const char* flattenLogs = flatten_logs();
    int currentOffset = 0;
    for (int i = 0; i < pageIndex; i++)
    {
        currentOffset += kconsole::calc_fit_substring(flattenLogs + currentOffset, MAX_SCREENSPACE);
    }

    int chunkLength = kconsole::calc_fit_substring(flattenLogs + currentOffset, MAX_SCREENSPACE);
    kconsole::print(flattenLogs + currentOffset, chunkLength);

    return currentOffset + chunkLength < (syslog_isWrapped ? SYSLOG_LENGTH : syslog_writePtr);
}
