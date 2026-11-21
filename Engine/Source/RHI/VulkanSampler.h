#pragma once
#include <Core/Core.h>
#include "VulkanContext.h"

namespace Bun::RHI
{
    class VulkanSampler
    {
    public:
        VulkanSampler() = default;
        BUN_NON_COPYABLE_MOVEABLE(VulkanSampler);

        void Init(VulkanContext& context);
        void Shutdown();

        VkSampler Get() const { return m_Sampler; }

    private:
        VulkanContext* m_Context = nullptr;
        VkSampler m_Sampler = VK_NULL_HANDLE;
    };
}