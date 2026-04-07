#include "bitmap_allocator.h"
#include "stdlib.h"
#include "math.h"

#define PAGE_SIZE 4096

void bitmap_allocator::set_bitmap_offset(size_t offset)
{
    m_bitmapOffset = offset;
}

void bitmap_allocator::init(void* spacePtr, size_t spaceSize)
{
    kernel_assert((size_t)spacePtr % 4096 == 0, "Space beginning not aligned.");
    
    m_bitmapData = (uint8*)spacePtr + m_bitmapOffset;
    m_bitmapDataLength = prepare_bitmap_space(spaceSize);
    
    mem_fill(m_bitmapData, 0, m_bitmapDataLength);

    m_begin = m_bitmapData + m_bitmapDataLength;
    if ((size_t)m_begin % 4096 > 0)
    {
        ((size_t&)m_begin) &= 0xFFFFF000;
        ((size_t&)m_begin) += 0x1000;
    }

    m_pagesCount = spaceSize / PAGE_SIZE;

    m_firstFreeBitmapByte = 0;
    m_usedPages = 0;
}

void* bitmap_allocator::alloc(size_t pageCount)
{
    if (pageCount == 0)
        return nullptr;

    bool hasFoundFirstFreeByte = false;
    
    size_t found = 0;
    size_t startPage = -1;
    for (size_t i = m_firstFreeBitmapByte; i < m_bitmapDataLength; i++)
    {
        if (found >= pageCount)
            break;
        
        uint8 cur = m_bitmapData[i];
        if (cur == 0xFF)
        {
            startPage = -1;
            found = 0;
            continue;
        }

        if (!hasFoundFirstFreeByte)
        {
            hasFoundFirstFreeByte = true;
            m_firstFreeBitmapByte = i;
        }

        for (size_t bit = 0; bit < 8; )
        {
            int firstClearBit = __builtin_ctz(~cur);
            if (firstClearBit != 0)
            {
                if (found >= pageCount)
                    break;
                
                found = 0;
                startPage = -1;

                cur >>= firstClearBit;
                bit += firstClearBit;
                continue; // Trigger the loop condition again.
            }

            if (startPage == -1)
                startPage = i * 8 + bit;
            
            if (cur == 0x00)
            {
                found += 8 - bit;
                break;
            }

            int zeroes = __builtin_ctz(cur);
            cur >>= zeroes;

            found += zeroes;
            bit += zeroes;
        }
    }

    if (found == 0)
        return nullptr;
    
    found = min(found, pageCount);
    found = min(found, (m_pagesCount - startPage) - 1);

    if (found < pageCount)
        return nullptr;

    mark_pages(startPage, pageCount, true);
    return m_begin + startPage * PAGE_SIZE;
}

void bitmap_allocator::free(void* pageAddr, size_t pageCount)
{
    if (pageCount == 0)
        return;
    
    kernel_assert((size_t)pageAddr % 4096 == 0, "Unaligned page address.");

    size_t diff = (uint8*)pageAddr - (uint8*)m_begin;
    size_t pageIndex = diff / PAGE_SIZE;

    kernel_assert(pageIndex < m_pagesCount, "Attempt to free a page that's not controlled by this allocator.");

    // uint8& bitmapByte = m_bitmapData[pageIndex / 8];
    // uint8 bitmapByteMask = 1 << (pageIndex % 8);

    // kernel_assert(bitmapByte & bitmapByteMask > 0, "Attempt to free a free page.");
    // bitmapByte &= ~bitmapByteMask;

    mark_pages(pageIndex, pageCount, false);

    size_t pageBitmapByte = pageIndex / 8;
    m_firstFreeBitmapByte = min(m_firstFreeBitmapByte, pageBitmapByte);
}

bool bitmap_allocator::belongs(void* addr) const
{
    void* end = (uint8*)m_begin + m_pagesCount * PAGE_SIZE;
    return addr >= m_begin && addr < end;
}

size_t bitmap_allocator::get_used_size() const
{
    return m_usedPages * PAGE_SIZE;
}

size_t bitmap_allocator::get_size() const
{
    return m_pagesCount * PAGE_SIZE;
}

size_t bitmap_allocator::prepare_bitmap_space(size_t spaceSize)
{
    size_t pages = spaceSize / PAGE_SIZE;
    size_t bitmapEstimatedLength = divide_round_up(pages, 8U);

    return bitmapEstimatedLength;
}

// size_t bitmap_allocator::check_free_sequence(size_t startPage, size_t count)
// {
//     size_t i = startPage / 8;
//     int firstClearBit = startPage & 8;

//     size_t foundPageSequenceCount = 0;
//     for (; i < m_bitmapDataLength; i++)
//     {
//         uint8& cur = m_bitmapData[i];
//         for (size_t bitIdx = firstClearBit; bitIdx < 8; bitIdx++)
//         {
//             if ((cur >> bitIdx) & 1 || (i * 8 + bitIdx) >= m_pagesCount)
//                 return foundPageSequenceCount;

//             foundPageSequenceCount++;
//             if (foundPageSequenceCount >= count)
//                 return foundPageSequenceCount;
//         }
        
//         firstClearBit = 0;
//     }

//     return foundPageSequenceCount;
// }

void bitmap_allocator::mark_pages(size_t startPage, size_t count, bool isUsed)
{
    size_t i = startPage / 8;
    int firstClearBit = startPage % 8;

    for (; i < m_bitmapDataLength; i++)
    {
        uint8& cur = m_bitmapData[i];
        for (size_t bitIdx = firstClearBit; bitIdx < 8; bitIdx++)
        {
            if (count == 0)
                return;
            
            if (isUsed)
                cur |= 1 << bitIdx;
            else
                cur &= ~(1 << bitIdx);

            count--;
            m_usedPages += isUsed ? 1 : -1;
        }
        
        firstClearBit = 0;
    }
}
