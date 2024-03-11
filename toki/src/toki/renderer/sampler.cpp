#include "sampler.h"

#include "renderer/vulkan_sampler.h"

namespace Toki {

Ref<Sampler> Sampler::create(const SamplerConfig& config) {
    return createRef<VulkanSampler>(config);
}

Sampler::Sampler(const SamplerConfig& config) : m_config(config) {}

}  // namespace Toki
