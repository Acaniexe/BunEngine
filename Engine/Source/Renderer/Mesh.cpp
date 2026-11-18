#include "Mesh.h"

namespace Bun::Renderer
{
    Mesh::Mesh() = default;
    
    void Mesh::Init(RHI::VulkanContext& context, const Assets::MeshData& data)
    {
        if (data.vertices.empty() || data.indices.empty()) return;

        m_IndexCount = (u32)data.indices.size();

        m_VertexBuffer.InitDeviceLocal(context, data.vertices.data(), sizeof(RHI::Vertex) * data.vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        m_IndexBuffer.InitDeviceLocal(context, data.indices.data(), sizeof(u32) * data.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }

    void Mesh::Shutdown()
    {
        if (m_IndexCount == 0) return;

        m_IndexBuffer.Shutdown();
        m_VertexBuffer.Shutdown();
        m_IndexCount = 0;
    }

    void Mesh::Bind(VkCommandBuffer cmd) const
    {
        VkBuffer vb = m_VertexBuffer.GetBuffer();
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &vb, &offset);
        vkCmdBindIndexBuffer(cmd, m_IndexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }

    void Mesh::Draw(VkCommandBuffer cmd) const
    {
        vkCmdDrawIndexed(cmd, m_IndexCount, 1, 0, 0, 0);
    }
}