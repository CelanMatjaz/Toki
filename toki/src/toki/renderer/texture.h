#pragma once

#include <filesystem>

#include "toki/core/core.h"
#include "toki/renderer/sampler.h"

namespace Toki {

struct TextureConfig {
    uint32_t width = 0, height = 0;
    Ref<Sampler> optionalSampler = nullptr;
};

class Texture {
public:
    static Ref<Texture> create(std::filesystem::path path, const TextureConfig& config);
    static Ref<Texture> create(const TextureConfig& config);

    Texture(std::filesystem::path path, const TextureConfig& config);
    Texture(const TextureConfig& config);
    virtual ~Texture() = default;

    virtual void setData(uint32_t size, void* data) = 0;

    void setOptionalSampler(Ref<Sampler> sampler);
    Ref<Sampler> getOptionalSampler() const;

protected:
    Texture() = default;

private:
    TextureConfig m_config;
};

}  // namespace Toki
