#pragma once

#include "toki/renderer/texture.h"

namespace Toki {

    class VulkanTexture : public Texture {
    public:

        VulkanTexture(const TextureConfig& config);
        ~VulkanTexture();

    private:
    };

}