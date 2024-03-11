#include "sampler.h"

#include "renderer/vulkan_sampler.h"

namespace Toki {

Ref<Sampler> Sampler::create(const SamplerConfig& config) {
    return createRef<VulkanSampler>(config);
}

Sampler::Sampler(const SamplerConfig& config) : m_config(config) {}

uint32_t Sampler::getSetIndex() const {
    return m_config.setIndex;
};
uint32_t Sampler::getBinding() const {
    return m_config.binding;
};
uint32_t Sampler::getArrayElementIndex() const {
    return m_config.arrayElement;
};

}  // namespace Toki
