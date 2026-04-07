#pragma once
#include "stdint.h"

class bitmap_allocator
{
private:
    size_t m_bitmapOffset = 0;

    uint8* m_bitmapData;
    size_t m_bitmapDataLength;

    void* m_begin;
    size_t m_pagesCount;

    size_t m_firstFreeBitmapByte;
    size_t m_usedPages;

public:
    void set_bitmap_offset(size_t offset);
    void init(void* spacePtr, size_t spaceSize);

    void* alloc(size_t pageCount);
    void free(void* pageAddr, size_t pageCount = 1);

    bool belongs(void* addr) const;

    size_t get_used_size() const;
    size_t get_size() const;

    /// @brief Pre-calculation for the given memory space.
    /// @return The amount of bytes required for the bitmap data.
    static size_t prepare_bitmap_space(size_t spaceSize);

private:
    void mark_pages(size_t startPage, size_t count, bool isUsed);
};
