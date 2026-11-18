#pragma once
#include <Core/Core.h>
#include "MeshData.h"

namespace Bun::Assets
{
    struct ImportedMaterial
    {
        std::string name;
        Math::Vec3 baseColor = Math::Vec3(0.8f);
        std::string diffuseTexture;
    };

    struct ImportedMesh
    {
        std::string name;
        MeshData data;
        int materialIndex = -1;
    };

    struct ImportedScene
    {
        std::vector<ImportedMesh> meshes;
        std::vector<ImportedMaterial> materials;
    };
}