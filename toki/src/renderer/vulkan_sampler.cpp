#include "vulkan_sampler.h"

#include "toki/core/assert.h"

namespace Toki {

VkSamplerAddressMode mapAddressMode(TextureRepeat r) {
    switch (r) {
        case TextureRepeat::Repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case TextureRepeat::Mirrored:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case TextureRepeat::ClampEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case TextureRepeat::ClampBorder:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:
            std::unreachable();
    }
}

VkFilter mapFilter(TextureFilter f) {
    switch (f) {
        case TextureFilter ::Linear:
            return VK_FILTER_LINEAR;
        case TextureFilter ::Nearest:
            return VK_FILTER_NEAREST;
        default:
            std::unreachable();
    }
};

VulkanSampler::VulkanSampler(const SamplerConfig& config) : Sampler(config) {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = mapFilter(config.magFilter);
    samplerInfo.minFilter = mapFilter(config.minFilter);
    samplerInfo.addressModeU = mapAddressMode(config.repeatU);
    samplerInfo.addressModeV = mapAddressMode(config.repeatV);
    samplerInfo.addressModeW = mapAddressMode(config.repeatW);
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
    int a = 0;
}

VulkanSampler::~VulkanSampler() {
    vkDestroySampler(s_context->device, m_sampler, s_context->allocationCallbacks);
}

VkSampler VulkanSampler::getSampler() const {
    return m_sampler;
}

}  // namespace Toki
