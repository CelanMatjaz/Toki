#pragma once

#include "core/core.h"
#include "filesystem"

namespace Toki {

    enum class TextureFormat {
        SINGLE, RGB, RGBA
    };

    struct TextureConfig {
        std::filesystem::path path;
        TextureFormat format = TextureFormat::RGBA;
    };

    class Texture {
    public:
        static Ref<Texture> create(const TextureConfig& config);

        virtual ~Texture() = default;
    };

}