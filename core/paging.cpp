#include "paging.h"

#include "stdlib.h"
#include "kernel.h"
#include "heap.h"
#include "isr.h"

#include "math.h"

namespace kpaging
{
    // static dir_entry_t* directory;

    extern "C" void paging_enable(directory* dir_pointer);

    static void page_fault_handler(const kisr::regs_t& regs);

    inline void invlpg(void* addr)
    {
        // i486 and later only.
        asm volatile("invlpg (%0)" :: "b"(addr) : "memory");
    }

    void init(directory* directory)
    {
        kisr::register_handler(14, page_fault_handler);

        kernel_log("Enabling paging...\n");
        paging_enable(directory);
    }

    void map_address(uint32 address)
    {
        // address &= 0xFFFFF000;
        // address /= 0x400000; // This is for 4MB pages.

        // set_dir_entry(address, true, true, address);
        // //invlpg((void*)address);
        
        RAISE_ERROR_D("Deprecated method call", "map_address(uint32) shouldn't be used anymore.");
    }

    void page_fault_handler(const kisr::regs_t& regs)
    {
        kconsole::clear();
        RAISE_ERROR("Page fault!");
    }
}
