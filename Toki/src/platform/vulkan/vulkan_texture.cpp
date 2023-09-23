#include "tkpch.h"
#include "vulkan_texture.h"
#include "core/assert.h"
#include "platform/vulkan/backend/vulkan_image.h"
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Toki {

    VulkanTexture::VulkanTexture(const TextureConfig& config) {
        int width, height, channels;
        int forcedChannels;
        VulkanImageConfig imageConfig{};

        switch (config.format) {
            case TextureFormat::SINGLE: {
                forcedChannels = STBI_grey;
                imageConfig.format = VK_FORMAT_R8_SRGB;
                break;
            }
            case TextureFormat::RGB: {
                forcedChannels = STBI_rgb;
                imageConfig.format = VK_FORMAT_R8G8B8_SRGB;
                break;
            }
            case TextureFormat::RGBA:
            default: {
                forcedChannels = STBI_rgb_alpha;
                imageConfig.format = VK_FORMAT_R8G8B8A8_SRGB;
                break;
            }
        }

        uint32_t* pixels = (uint32_t*) stbi_load(config.path.string().c_str(), &width, &height, &channels, forcedChannels);
        VkDeviceSize imageSize = width * height * forcedChannels;

        TK_ASSERT(pixels != nullptr, "");
        TK_ASSERT(imageSize > 0, "");

        imageConfig.extent = { (uint32_t) width, (uint32_t) height, 1 };
        imageConfig.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        imageHandle = createRef<VulkanImage>(imageConfig);
        imageHandle->setData(imageSize, pixels);

        stbi_image_free(pixels);
    }

    VulkanTexture::VulkanTexture(Ref<VulkanImage> image) : imageHandle(image) {

    }

    VulkanTexture::~VulkanTexture() {

    }

}