#pragma once

#include "toki/renderer/texture.h"
#include "platform/vulkan/backend/vulkan_image.h"

namespace Toki {

    class VulkanTexture : public Texture {
    public:

        VulkanTexture(const TextureConfig& config);
        VulkanTexture(Ref<VulkanImage> image);
        ~VulkanTexture();

        Ref<VulkanImage> getHandle() { return imageHandle; }

    private:
        VkFormat format;

        Ref<VulkanImage> imageHandle;
    };

}