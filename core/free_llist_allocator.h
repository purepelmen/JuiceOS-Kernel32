#pragma once
#include "stdint.h"

void heap_free_llist_init(void* location, size_t heapSize);
void* heap_free_llist_alloc(size_t size);
void heap_free_llist_free(void* addr);

size_t heap_free_llist_get_unused();
void heap_free_llist_dump_free_blocks();
