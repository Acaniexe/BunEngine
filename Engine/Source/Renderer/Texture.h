#pragma once
#include <Core/Core.h>
#include <RHI/RHI.h>
#include <Assets/ImageData.h>

namespace Bun::Renderer
{
    class Texture
    {
    public:
        Texture() = default;

        void Init(RHI::VulkanContext& context, RHI::VulkanSampler& sampler, const Assets::ImageData& image);
        void Shutdown();

        VkDescriptorSet GetSet() const { return m_Descriptor.GetSet(); }
        VkDescriptorSetLayout GetLayout() const { return m_Descriptor.GetLayout(); }

    private:
        RHI::VulkanImage m_Image;
        RHI::VulkanDescriptor m_Descriptor;
        bool m_Valid = false;
    };
}