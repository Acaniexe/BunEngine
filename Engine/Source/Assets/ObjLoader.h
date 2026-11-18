#pragma once
#include "ImportedScene.h"
#include <Core/Core.h>

namespace Bun::Assets
{
    class ObjLoader
    {
    public:
        static bool Load(const std::string& path, ImportedScene& out);
    };
}