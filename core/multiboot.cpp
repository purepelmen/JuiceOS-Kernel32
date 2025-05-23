#include "multiboot.h"

multiboot_tag* multiboot_traverse_tag(multiboot_tag* tag)
{
    // Skips the size (tag->size) + makes address 8-bytes aligned.
    uint32 nextTagAddr = ((uint32)tag + tag->size + 7) & ~7;
    return (multiboot_tag*)nextTagAddr;
}

multiboot_tag* multiboot_find_tag(multiboot_tag* startTag, multiboot_tagtype type)
{
    multiboot_tag* current = startTag;
    while (current->type != type && !current->is_end())
    {
        current = multiboot_traverse_tag(current);
    }

    return current;
}
