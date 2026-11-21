#pragma once
#include <Core/Core.h>

namespace Bun::Assets
{
    struct ImageData
    {
        u32 width = 0;
        u32 height = 0;
        std::vector<u8> pixels;
    };
}