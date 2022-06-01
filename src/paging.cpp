#include "drivers/screen.hpp"
#include "paging.hpp"
#include "stdlib.hpp"
#include "heap.hpp"
#include "isr.hpp"

namespace kpaging
{
    dir_entry_t* directory;

    extern "C" void paging_enable(uint32 dir_pointer);

    static void set_dir_entry(int index, uint8 writable, uint8 four_mb_page, uint32 address);
    static void page_fault_handler(kisr::isr_regs_t regs);

    void paging_init()
    {
        directory = (dir_entry_t*) malloc_pg_aligned(sizeof(dir_entry_t) * 1024);
        mem_fill((uint8*) directory, 0, sizeof(dir_entry_t) * 1024);

        set_dir_entry(0, true, true, 0x0);

        kisr::register_handler(14, page_fault_handler);
        paging_enable((uint32) directory);
    }

    static void set_dir_entry(int index, uint8 writable, uint8 four_mb_page, uint32 address)
    {
        if(four_mb_page)
        {
            address = (address & 0x3FF) << 10;
        }

        directory[index].present = true;
        directory[index].writable = writable;
        directory[index].usermode_accessable = true;
        directory[index].write_through_caching = false;
        directory[index].caching_disabled = false;
        directory[index].four_mb_pages = four_mb_page;
        directory[index].global = false;
        directory[index].address = address;
    }

    static void page_fault_handler(kisr::isr_regs_t regs)
    {
        kscreen::clear();
        RAISE_ERROR("Page fault!");
    }
}
