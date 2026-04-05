#pragma once
#include "stdint.h"

struct llist_free_entry;

class fll_allocator
{
private:
    llist_free_entry* m_first;
    size_t m_heapSize = 0;

public:
    void init(void* location, size_t heapSize);

    void* alloc(size_t size);
    void free(void* addr);

    size_t get_unused() const;
    size_t get_heap_size() const;

    void dump() const;

private:
    void llist_insert_before(llist_free_entry* target, void* address, size_t size);
    void llist_insert_after(llist_free_entry* target, void* address, size_t size);
    void llist_remove(llist_free_entry* entry);

    size_t llist_shrink(llist_free_entry* block, size_t amount);
    void llist_grow_up(llist_free_entry* entry, size_t amount);
};
