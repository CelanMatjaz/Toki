#include "vulkan_sampler.h"

#include "toki/core/assert.h"

namespace Toki {

VulkanSampler::VulkanSampler(const SamplerConfig& config) : Sampler(config) {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(s_context->physicalDevice, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    TK_ASSERT_VK_RESULT(vkCreateSampler(s_context->device, &samplerInfo, s_context->allocationCallbacks, &m_sampler), "Could not create sampler");
}

VulkanSampler::~VulkanSampler() {
    vkDestroySampler(s_context->device, m_sampler, s_context->allocationCallbacks);
}

VkSampler VulkanSampler::getSampler() const {
    return m_sampler;
}

}  // namespace Toki
