#include "mbr.h"

namespace kpart
{
    MbrPartitionObject::MbrPartitionObject(void *pointer, uint8 index) : pointer((mbr_partition*)pointer), index(index)
    {
    }

    MbrPartitionObject MbrObject::GetPartition(uint8 index)
    {
        if (index > 3)
        {
            // Error handling.    
        }

        return MbrPartitionObject(&pointer->partitions[index], index);
    }

    MbrObject::MbrObject(void* pointer) : pointer((mbr*)pointer)
    {
    }
}
