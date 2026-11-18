#pragma once
#include <Core/Core.h>
#include <RHI/Vertex.h>
#include <vector>

namespace Bun::Assets
{
    struct MeshData
    {
        std::vector<RHI::Vertex> vertices;
        std::vector<u32> indices;
    };
}