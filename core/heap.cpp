#include "stdlib.h"
#include "kernel.h"
#include "heap.h"
#include "math.h"
#include "paging.h"

#include "free_llist_allocator.h"
#include "bitmap_allocator.h"

#define IMAGE_BASE 0x100000

extern uint32 end;
extern uint32 stack_base_addr;

/* The heap.h new[] template function uses it, because sometimes new[] requires the definition of this. */
extern "C" void __cxa_throw_bad_array_new_length()
{
    RAISE_ERROR("__cxa_throw_bad_array_new_length happened")
}

namespace kheap
{
    const size_t HEAP_SIZE = 1024 * 64;

    static uint8* dynmemStart;
    static kpaging::directory* kernelPagingDir;

    static bitmap_allocator pageAllocator;
    static fll_allocator allocator;

    static void paging_map(void* addr, size_t count);
    static void paging_unmap(void* addr, size_t count);

    void init()
    {
        mmap_entry entry = kernel_get_mmap();
        while (!entry.is_end())
        {
            if (!entry.is_valid_addr_ptr())
                continue;
            
            if (entry.is_available() && entry.get_addr_ptr() >= (void*)IMAGE_BASE)
                break;

            entry = entry.next();
        }

        kernel_assert(!entry.is_end());

        dynmemStart = (uint8*)max((void*)&end, entry.get_addr_ptr());

        pageAllocator.set_bitmap_offset(dynmemStart - (uint8*)entry.get_addr_ptr());
        pageAllocator.init(entry.get_addr_ptr(), entry.get_length());

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
        
        // Now preparation of heap.
        void* heapMemory = pageAllocator.alloc(4);
        allocator.init(heapMemory, 4096 * 4);
        kernel_log("Dynamic memory initialized (heap size: %d KB).\n", HEAP_SIZE / 1024);

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

    void* alloc(size_t size)
    {
        return allocator.alloc(size);
    }

    void free(void* ptr)
    {
        allocator.free(ptr);
    }

    void print_alloc_free_blocks()
    {
        allocator.dump();
    }

    uint32 get_allocated()
    {
        return pageAllocator.get_used_size();
        // return (uint32) (dynmemPtr - dynmemStart);
    }

    uint32 get_kernel_memsize()
    {
        return (uint32) ((uint8*)&end - IMAGE_BASE);
    }

    uint32 get_heap_free()
    {
        return allocator.get_unused();
    }

    uint32 get_heap_size()
    {
        return allocator.get_heap_size();
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
                tableEntry->address = (uint32)(addr + i * 4096) >> 12;

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
