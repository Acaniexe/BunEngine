#pragma once
#include <Core/Core.h>
#include "ImageData.h"

namespace Bun::Assets
{
    class ImageLoader
    {
    public:
        static bool Load(const std::string& path, ImageData& out);
    };
}