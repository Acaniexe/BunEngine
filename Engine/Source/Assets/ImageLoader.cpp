#include "ImageLoader.h"

#if defined(_MSC_VER)
    #pragma warning(push, 0)
#elif defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    #pragma GCC diagnostic ignored "-Wunused-function"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#if defined(_MSC_VER)
    #pragma warning(pop)
#elif defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

namespace Bun::Assets
{
    bool ImageLoader::Load(const std::string& path, ImageData& out)
    {
        int w = 0, h = 0, channels = 0;
        stbi_uc* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
        if (!data) return false;

        out.width = (u32)w;
        out.height = (u32)h;
        out.pixels.assign(data, data + (usize)w * (usize)h * 4);

        stbi_image_free(data);
        return true;
    }
}