#pragma once
#include <drivers/storage.h>

namespace kcd
{
    class ISO9660
    {
    };

    kstorage::FileSystem* probe(kstorage::BlockDevice* drive);
}