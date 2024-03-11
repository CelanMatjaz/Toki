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

void Texture::setOptionalSampler(Ref<Sampler> sampler) {
    m_config.optionalSampler = sampler;
}

Ref<Sampler> Texture::getOptionalSampler() const {
    return m_config.optionalSampler;
}

}  // namespace Toki
