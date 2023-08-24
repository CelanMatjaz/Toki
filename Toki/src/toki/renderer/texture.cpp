#include "tkpch.h"
#include "texture.h"
#include "renderer.h"

#include "platform/vulkan/vulkan_texture.h"

namespace Toki {

    Ref<Texture> Texture::create(const TextureConfig& config) {
        return createRef<VulkanTexture>(config);
    }

}