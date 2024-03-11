#pragma once

#include "toki/renderer/sampler.h"
#include "vulkan_types.h"

namespace Toki {
class VulkanRenderer;

class VulkanSampler : public Sampler {
    friend VulkanRenderer;

public:
    VulkanSampler(const SamplerConfig& config);
    ~VulkanSampler();

    VkSampler getSampler() const;

private:
    VkSampler m_sampler = VK_NULL_HANDLE;

    inline static Ref<VulkanContext> s_context;
};

}  // namespace Toki
