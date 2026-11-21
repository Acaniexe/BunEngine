#include "Texture.h"

namespace Bun::Renderer
{
    void Texture::Init(RHI::VulkanContext& context, RHI::VulkanSampler& sampler, const Assets::ImageData& image)
    {
        if (image.pixels.empty() || image.width == 0 || image.height == 0) return;

        m_Image.Init(context, image.width, image.height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
        m_Image.UploadPixels(image.pixels.data(), image.pixels.size());

        m_Descriptor.Init(context);
        m_Descriptor.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
        m_Descriptor.Build();
        m_Descriptor.UpdateImage(0, m_Image.GetView(), sampler.Get());
        m_Valid = true;
    }

    void Texture::Shutdown()
    {
        if (!m_Valid) return;
        m_Descriptor.Shutdown();
        m_Image.Shutdown();
        m_Valid = false;
    }
}