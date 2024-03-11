#pragma once

#include <filesystem>

#include "toki/core/core.h"

namespace Toki {

struct TextureConfig {
    uint32_t setIndex = 0;
    uint32_t binding = 0;
    uint32_t arrayElement = 0;
};

class Texture {
public:
    static Ref<Texture> create(std::filesystem::path path, const TextureConfig& config);
    static Ref<Texture> create(const TextureConfig& config);

    Texture(std::filesystem::path path, const TextureConfig& config);
    Texture(const TextureConfig& config);
    virtual ~Texture() = default;

    virtual void setData(uint32_t size, void* data) = 0;

    uint32_t getSetIndex() const;
    uint32_t getBinding() const;
    uint32_t getArrayElementIndex() const;
protected:

    Texture() = default;
private:
    TextureConfig m_config;
};

}  // namespace Toki
