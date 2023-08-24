#pragma once

#include "core/core.h"

namespace Toki {

    struct TextureConfig {};

    class Texture {
    public:
        static Ref<Texture> create(const TextureConfig& config);

        virtual ~Texture() = default;
    };

}