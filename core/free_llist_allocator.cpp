#include "free_llist_allocator.h"

#include "math.h"
#include "stdlib.h"
#include "console.h"

struct llist_free_entry
{
    llist_free_entry* prev = nullptr;
    llist_free_entry* next = nullptr;

    size_t size;
};

const uint32 ALLOC_HEADER_CHECKSUM = 0xCAFE1234;

struct alloc_header
{
    uint32 checksum;
    size_t size;
};

static llist_free_entry* freeListFirst = nullptr;

static void llist_insert_before(llist_free_entry* target, void* address, size_t size);
static void llist_insert_after(llist_free_entry* target, void* address, size_t size);
static void llist_remove(llist_free_entry* entry);

static size_t llist_shrink(llist_free_entry* entry, size_t amount);
static void llist_grow_up(llist_free_entry* entry, size_t amount);

void heap_free_llist_init(void* location, size_t heapSize)
{
    kernel_assert(heapSize >= sizeof(llist_free_entry));

    freeListFirst = (llist_free_entry*)location;
    freeListFirst->size = heapSize;
    freeListFirst->prev = freeListFirst->next = nullptr;
}

void* heap_free_llist_alloc(size_t size)
{
    if (size == 0)
        return nullptr;
    
    size += sizeof(alloc_header);
    size = max(size, sizeof(llist_free_entry));

    llist_free_entry* suitable = freeListFirst;
    while (suitable->size < size)
    {
        suitable = suitable->next;
    }

    kernel_assert(suitable != nullptr, "Can't allocate more heap, no suitable free block of size %d.", size - sizeof(alloc_header));

    void* addr = suitable;
    size = llist_shrink(suitable, size);

    alloc_header* header = (alloc_header*)addr;
    header->checksum = ALLOC_HEADER_CHECKSUM;
    header->size = size;

    return addr + sizeof(alloc_header);
}

void heap_free_llist_free(void* addr)
{
    addr = addr - sizeof(alloc_header);
    alloc_header* header = (alloc_header*)addr;

    kernel_assert(header->checksum == ALLOC_HEADER_CHECKSUM && header->size > sizeof(alloc_header));
    header->checksum = 0;

    llist_free_entry* entry = freeListFirst;
    if (entry == nullptr)
    {
        llist_free_entry* newEntry = (llist_free_entry*)addr;
        newEntry->size = header->size;

        freeListFirst = newEntry;
        return;
    }

    while (entry != nullptr)
    {
        int topDistance = (size_t)entry - ((size_t)addr + header->size);
        int bottomDistance = (size_t)addr - ((size_t)entry + entry->size);

        kernel_assert(!(topDistance < 0 && topDistance >= -header->size), "Freeing area overlaps with already free data. May be a double-free.");
        kernel_assert(!(bottomDistance < 0 && bottomDistance >= -header->size), "Freeing area overlaps with already free data. May be a double-free.");

        if (bottomDistance == 0)
        {
            entry->size += header->size;
            if (entry->next != nullptr && ((size_t)entry->next - ((size_t)entry + entry->size)) == 0)
            {
                entry->size += entry->next->size;
                llist_remove(entry->next);
            }

            break;
        }

        if (topDistance >= 0)
        {
            if (topDistance == 0)
                llist_grow_up(entry, header->size);
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

    kernel_assert(entry != nullptr, "heap_free_llist_free() impossible branch reached. Failed to find a suitable method to reclaim memory.");
}

size_t heap_free_llist_get_unused()
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

void heap_free_llist_dump_free_blocks()
{
    llist_free_entry* entry = freeListFirst;
    do
    {
        kconsole::printf("%p, sized: %d B\n", entry, entry->size);
        entry = entry->next;
    } 
    while (entry != nullptr);
}

void llist_insert_before(llist_free_entry* target, void* address, size_t size)
{
    kernel_assert(size >= sizeof(llist_free_entry));

    llist_free_entry* newEntry = (llist_free_entry*)address;
    newEntry->size = size;

    if (freeListFirst == target)
        freeListFirst = newEntry;
    else
        target->prev->next = newEntry;
    
    newEntry->prev = target->prev;
    newEntry->next = target;
    target->prev = newEntry;
}

void llist_insert_after(llist_free_entry* target, void* address, size_t size)
{
    kernel_assert(size >= sizeof(llist_free_entry));

    llist_free_entry* newEntry = (llist_free_entry*)address;
    newEntry->size = size;

    if (target->next != nullptr)
    {
        target->next->prev = newEntry;
    }
    newEntry->prev = target;
    newEntry->next = target->next;
    target->next = newEntry;
}

void llist_remove(llist_free_entry* entry)
{
    if (freeListFirst == entry)
        freeListFirst = entry->next;

    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;

    entry->size = 0;
}

size_t llist_shrink(llist_free_entry* block, size_t amount)
{
    kernel_assert(block->size >= amount);

    size_t left = block->size - amount;
    if (left < sizeof(llist_free_entry))
    {
        size_t blockSize = block->size;
        llist_remove(block);

        return blockSize;
    }

    llist_free_entry* newPlace = (llist_free_entry*)((uint8*)block + amount);
    *newPlace = *block;
    
    newPlace->size = left;
    newPlace->prev->next = newPlace;
    newPlace->next->prev = newPlace;

    if (block == freeListFirst)
        freeListFirst = newPlace;
    return amount;
}

void llist_grow_up(llist_free_entry* entry, size_t amount)
{
    llist_free_entry* newPlace = (llist_free_entry*)((uint8*)entry - amount);
    *newPlace = *entry;
    
    newPlace->size += amount;
    newPlace->prev->next = newPlace;
    newPlace->next->prev = newPlace;

    if (entry == freeListFirst)
        freeListFirst = newPlace;
}
