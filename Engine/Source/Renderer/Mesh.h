#pragma once
#include <Core/Core.h>
#include <RHI/RHI.h>
#include <Assets/MeshData.h>

namespace Bun::Renderer
{
    class Mesh
    {
    public:
        Mesh();
        BUN_NON_COPYABLE_MOVEABLE(Mesh);

        void Init(RHI::VulkanContext& context, const Assets::MeshData& data);
        void Shutdown();

        void Bind(VkCommandBuffer cmd) const;
        void Draw(VkCommandBuffer cmd) const;

    private:
        RHI::VulkanBuffer m_VertexBuffer;
        RHI::VulkanBuffer m_IndexBuffer;
        u32 m_IndexCount = 0;
    };
}