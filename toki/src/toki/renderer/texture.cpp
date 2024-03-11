#include "texture.h"

#include "renderer/vulkan_image.h"

namespace Toki {

Ref<Texture> Texture::create(std::filesystem::path path, const TextureConfig& config) {
    return createRef<VulkanImage>(path, config);
}

Ref<Texture> Texture::create(const TextureConfig& config) {
    return nullptr;
}

Texture::Texture(std::filesystem::path path, const TextureConfig& config) : m_config(config) {}

Texture::Texture(const TextureConfig& config) {}

uint32_t Texture::getSetIndex() const {
    return m_config.setIndex;
};
uint32_t Texture::getBinding() const {
    return m_config.binding;
};
uint32_t Texture::getArrayElementIndex() const {
    return m_config.arrayElement;
};

Ref<Sampler> Texture::getOptionalSampler() const {
    return m_config.optionalSampler;
}

}  // namespace Toki
