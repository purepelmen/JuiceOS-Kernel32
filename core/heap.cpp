#include "stdlib.h"
#include "string.h"
#include "kernel.h"
#include "heap.h"

extern uint32 end;
extern uint32 stack_base_addr;

/* The heap.h new[] template function uses it, because sometimes new[] requires the definition of this. */
extern "C" void __cxa_throw_bad_array_new_length()
{
    RAISE_ERROR("__cxa_throw_bad_array_new_length happened")
}

struct llist_free_entry
{
    llist_free_entry* prev = nullptr;
    llist_free_entry* next = nullptr;

    void* address;
    size_t size;

    void shrink(size_t amount)
    {
        address = address + amount;
        size -= amount;
    }

    void growUp(size_t amount)
    {
        address = address - amount;
        size += amount;
    }

    bool is_empty() const { return address == nullptr; }
};

const uint32 ALLOC_HEADER_CHECKSUM = 0xDEAD1234;
const uint32 ALLOC_HEADER_FREED_CHECKSUM = 0xDEADDEAD;

struct alloc_header
{
    uint32 checksum;
    size_t size;
};

const size_t FREE_LIST_MAXSIZE = 256;
static llist_free_entry freeList[FREE_LIST_MAXSIZE];
static llist_free_entry* freeListFirst = nullptr;

void llist_insert_before(llist_free_entry* target, void* address, size_t size)
{
    llist_free_entry* newEntry = nullptr;
    for (size_t i = 0; i < FREE_LIST_MAXSIZE; i++)
    {
        if (freeList[i].is_empty())
            newEntry = freeList + i;
    }

    kernel_assert(newEntry != nullptr, "No available entries in the free linked list.");

    if (freeListFirst == target)
        freeListFirst = newEntry;
    else
        target->prev->next = newEntry;
    
    newEntry->prev = target->prev;
    newEntry->next = target;
    target->prev = newEntry;

    newEntry->address = address;
    newEntry->size = size;
}

void llist_insert_after(llist_free_entry* target, void* address, size_t size)
{
    llist_free_entry* newEntry = nullptr;
    for (size_t i = 0; i < FREE_LIST_MAXSIZE; i++)
    {
        if (freeList[i].is_empty())
            newEntry = freeList + i;
    }

    kernel_assert(newEntry != nullptr, "No available entries in the free linked list.");

    if (target->next != nullptr)
    {
        target->next->prev = newEntry;
    }
    newEntry->prev = target;
    newEntry->next = target->next;
    target->next = newEntry;

    newEntry->address = address;
    newEntry->size = size;
}

void llist_remove(llist_free_entry* entry)
{
    if (freeListFirst == entry)
        freeListFirst = entry->next;

    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;

    entry->address = nullptr;
    entry->size = 0;
}

void heap_free_llist_init(void* location, size_t heapSize)
{
    llist_free_entry allHeap{};
    allHeap.address = location;
    allHeap.size = heapSize;
    allHeap.prev = allHeap.next = nullptr;

    freeList[0] = allHeap;
    freeListFirst = freeList;
}

void* heap_free_llist_alloc(size_t size)
{
    if (size == 0)
        return nullptr;
    
    size += sizeof(alloc_header);

    llist_free_entry* suitable = freeListFirst;
    while (suitable->size < size)
    {
        suitable = suitable->next;
    }

    kernel_assert(suitable != nullptr, "Can't allocate more heap, no suitable free block of size %d.", size - sizeof(alloc_header));

    void* addr = suitable->address;
    suitable->shrink(size);

    if (suitable->size == 0)
        llist_remove(suitable);

    alloc_header* header = (alloc_header*)addr;
    header->checksum = ALLOC_HEADER_CHECKSUM;
    header->size = size;

    return addr + sizeof(alloc_header);
}

void heap_free_llist_free(void* addr)
{
    addr = addr - sizeof(alloc_header);
    alloc_header* header = (alloc_header*)addr;

    kernel_assert(header->checksum != ALLOC_HEADER_FREED_CHECKSUM, "Freeing an already freed memory.");
    kernel_assert(header->checksum == ALLOC_HEADER_CHECKSUM && header->size > sizeof(alloc_header));

    llist_free_entry* entry = freeListFirst;
    do
    {
        int topDistance = (size_t)entry->address - ((size_t)addr + header->size);
        int bottomDistance = (size_t)addr - ((size_t)entry->address + entry->size);

        kernel_assert(!(topDistance < 0 && topDistance >= -header->size), "Freeing area overlaps with already free data. May be a double-free.");
        kernel_assert(!(bottomDistance < 0 && bottomDistance >= -header->size), "Freeing area overlaps with already free data. May be a double-free.");

        if (bottomDistance == 0)
        {
            entry->size += header->size;
            if (entry->next != nullptr && ((size_t)entry->next->address - ((size_t)entry->address + entry->size)) == 0)
            {
                entry->size += entry->next->size;
                llist_remove(entry->next);
            }

            break;
        }

        if (topDistance >= 0)
        {
            if (topDistance == 0)
                entry->growUp(header->size);
            else
                llist_insert_before(entry, addr, header->size);
            
            break;
        }

        if (bottomDistance > 0 && entry->next == nullptr)
        {
            llist_insert_after(entry, addr, header->size);
            break;
        }

        entry = entry->next;
    } 
    while (entry != nullptr);

    kernel_assert(entry != nullptr, "heap_free_llist_free() impossible branch reached. Failed to find a suitable method to reclaim memory.");

    // This is usefull for debugging and error checking, so we can surely know if it's a freed memory portion.
    header->checksum = ALLOC_HEADER_FREED_CHECKSUM;
}

namespace kheap
{
    const size_t HEAP_SIZE = 1024 * 64;

    static uint8* dynmemStart;
    static uint8* dynmemPtr;

    void init()
    {
        // heap_start_value = (uint8*)&end + (1024 * 64);
        dynmemStart = (uint8*)&end;
        dynmemPtr = dynmemStart;

        void* heapMemory = sbrk(HEAP_SIZE);
        heap_free_llist_init(heapMemory, HEAP_SIZE);

        kernel_log("Dynamic memory initialized (heap size: %d KB).\n", HEAP_SIZE / 1024);
    }

    void reset()
    {
        dynmemPtr = dynmemStart;

        void* heapMemory = sbrk(HEAP_SIZE);
        heap_free_llist_init(heapMemory, HEAP_SIZE);

        kernel_log("Heap resetting has been completed. This will break references.\n");
    }

    void* sbrk(size_t size)
    {
        uint8* tmp = dynmemPtr;
        dynmemPtr += size;

        return tmp;
    }

    void* sbrk_pgaligned(size_t size)
    {
        uint32 currentPtr = (uint32)dynmemPtr;
        if(currentPtr & 0xFFFFF000)
        {
            currentPtr &= 0xFFFFF000;
            currentPtr += 0x1000;
        }
        dynmemPtr = (uint8*) currentPtr;

        return sbrk(size);
    }

    void* alloc(size_t size)
    {
        return heap_free_llist_alloc(size);
    }

    void free(void* ptr)
    {
        heap_free_llist_free(ptr);
    }

    void print_alloc_free_blocks()
    {
        llist_free_entry* entry = freeListFirst;
        do
        {
            kconsole::printf("%p, sized: %d B\n", entry->address, entry->size);
            entry = entry->next;
        } 
        while (entry != nullptr);
    }

    uint32 get_allocated()
    {
        return (uint32) (dynmemPtr - dynmemStart);
    }

    uint32 get_kernel_memsize()
    {
        return (uint32) (dynmemStart - 0x100000);
    }

    uint32 get_heap_free()
    {
        size_t counter = 0;

        llist_free_entry* entry = freeListFirst;
        do
        {
            counter += entry->size;
            entry = entry->next;
        } 
        while (entry != nullptr);

        return counter;
    }

    uint32 get_heap_size()
    {
        return HEAP_SIZE;
    }

    uint32 get_stack_usage()
    {
        uint32 esp;
        asm volatile ("movl %%esp, %0" : "=r" (esp));

        return stack_base_addr - esp;
    }

    void *get_location_ptr()
    {
        return dynmemStart;
    }
}
