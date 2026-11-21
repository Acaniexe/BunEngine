#include "VulkanSampler.h"
#include <Core/Logger.h>

namespace Bun::RHI
{
    void VulkanSampler::Init(VulkanContext& context)
    {
        m_Context = &context;
        
        VkSamplerCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_LINEAR;
        info.minFilter = VK_FILTER_LINEAR;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.maxAnisotropy = 8.0f;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.minLod = 0.0f;
        info.maxLod = 0.0f;
        info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

        if (vkCreateSampler(m_Context->GetDevice(), &info, nullptr, &m_Sampler) != VK_SUCCESS)
            BUN_FATAL("Failed to create sampler!");
    }

    void VulkanSampler::Shutdown()
    {
        if (m_Sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(m_Context->GetDevice(), m_Sampler, nullptr);
            m_Sampler = VK_NULL_HANDLE;
        }
    }
}