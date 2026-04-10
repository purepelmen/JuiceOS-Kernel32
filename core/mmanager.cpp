#include "mmanager.h"
#include "stdlib.h"
#include "kernel.h"
#include "math.h"
#include "paging.h"

#include "bitmap_allocator.h"

#define IMAGE_BASE 0x100000

extern uint32 end;
extern uint32 stack_base_addr;

namespace kmmanager
{
    static uint8* dynmemStart;
    static kpaging::directory* kernelPagingDir;

    static bitmap_allocator pageAllocator;

    static void paging_map(void* addr, size_t count);
    static void paging_unmap(void* addr, size_t count);

    void init()
    {
        const mmap_entry* mainEntry = nullptr;

        mmap_list list = kernel_get_mmap();
        for (const mmap_entry& entry: list)
        {
            if (!entry.is_valid_addr_ptr())
                continue;

            if (entry.isAvailable && entry.get_addr_ptr() >= (void*)IMAGE_BASE)
            {
                mainEntry = &entry;
                break;
            }
        }
        kernel_assert(mainEntry != nullptr);

        dynmemStart = (uint8*)max((void*)&end, mainEntry->get_addr_ptr());

        pageAllocator.set_bitmap_offset(dynmemStart - (uint8*)mainEntry->get_addr_ptr());
        pageAllocator.init(mainEntry->get_addr_ptr(), mainEntry->length);

        // Setting up the main paging directory for kernel.
        kernelPagingDir = (kpaging::directory*)pageAllocator.alloc(divide_round_up(sizeof(kpaging::directory), 4096U));
        mem_fill(kernelPagingDir, 0, sizeof(kpaging::directory));

        // Mapping the first 4MB including the kernel.
        auto& imageDirEntry = kernelPagingDir->entries[IMAGE_BASE >> 22];
        if (true)
        {
            imageDirEntry.present = true;
            imageDirEntry.writable = true;
            imageDirEntry.usermode_accessable = false;
            imageDirEntry.four_mb_pages = true;

            imageDirEntry.to_4MB.address_31_22 = (uint32)IMAGE_BASE >> 22;
        }

        kernel_assert(end < 0x400000, "The kernel exceeds first 4MB of memory.");

        // Finally setup paging (enable CR0's paging and PSE bits, setup CR3, setup ISR).
        kpaging::init(kernelPagingDir);
    }

    void* alloc_pages(size_t amount)
    {
        void* allocated = pageAllocator.alloc(amount);
        paging_map(allocated, amount);

        return allocated;
    }

    void free_pages(void* addr, size_t count)
    {
        kernel_assert(pageAllocator.belongs(addr));

        paging_unmap(addr, count);
        pageAllocator.free(addr, count);
    }

    uint32 get_allocated()
    {
        return pageAllocator.get_used_size();
    }

    uint32 get_kernel_memsize()
    {
        return (uint32) ((uint8*)&end - IMAGE_BASE);
    }

    uint32 get_stack_usage()
    {
        uint32 esp;
        asm volatile ("movl %%esp, %0" : "=r" (esp));

        return stack_base_addr - esp;
    }

    void* get_location_ptr()
    {
        return dynmemStart;
    }

    size_t get_page_count_for(size_t sizeInBytes)
    {
        return divide_round_up(sizeInBytes, 4096U);
    }

    void paging_map(void* addr, size_t pageCount)
    {
        auto decomposed = kpaging::addr::from(addr);

        auto& dirEntry = kernelPagingDir->entries[decomposed.dirEntry];
        if (!dirEntry.present)
        {
            void* newPageTable = pageAllocator.alloc(divide_round_up(sizeof(kpaging::page_table), 4096U));
            mem_fill(newPageTable, 0, sizeof(kpaging::page_table));

            dirEntry.present = true;
            dirEntry.writable = true;
            dirEntry.usermode_accessable = false;
            dirEntry.four_mb_pages = false;

            dirEntry.to_pagetable.address = (uint32)newPageTable >> 12;
        }

        if (!dirEntry.four_mb_pages)
        {
            kpaging::page_table* pageTable = (kpaging::page_table*)(dirEntry.to_pagetable.address << 12);
            auto tableEntry = &pageTable->entries[decomposed.tableEntry];

            kernel_assert(pageCount < 1024 - decomposed.tableEntry, "Can't map %d pages because there's no more space in the page table.", pageCount);
            for (size_t i = 0; i < pageCount; i++)
            {
                kernel_assert(!tableEntry->present);

                tableEntry->present = true;
                tableEntry->read_write = true;
                tableEntry->user_mode = false;
                tableEntry->address = ((nptr_t)addr + i * 4096) >> 12;

                tableEntry++;
            }
        }
    }

    void paging_unmap(void* addr, size_t count)
    {
        auto decomp = kpaging::addr::from(addr);

        auto& dirEntry = kernelPagingDir->entries[decomp.dirEntry];
        kernel_assert(dirEntry.present);

        if (!dirEntry.four_mb_pages)
        {
            kpaging::page_table* pageTable = (kpaging::page_table*)(dirEntry.to_pagetable.address << 12);
            auto tableEntry = &pageTable->entries[decomp.tableEntry];

            kernel_assert(count < 1024 - decomp.tableEntry, "Can't unmap %d pages because there's no more space in the page table.", count);
            for (size_t i = 0; i < count; i++)
            {
                kernel_assert(!tableEntry->present);

                tableEntry->present = false;
                tableEntry++;
            }
        }
    }
}
