#pragma once

#include <vulkan/vulkan.h>

namespace Toki {

struct Sampler {
    Sampler();
    ~Sampler();

    VkSampler m_sampler;
};

}  // namespace Toki
