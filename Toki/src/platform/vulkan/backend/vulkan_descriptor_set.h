#pragma once

#include "vulkan/vulkan.h"
#include "unordered_map"
#include "platform/vulkan/vulkan_buffer.h"
#include "platform/vulkan/vulkan_texture.h"

namespace Toki {

    enum class DescriptorSetBindingType {
        Uniform, Sampler2D
    };

    struct DescriptorSetBinding {
        DescriptorSetBindingType type;
        uint32_t size = 1;
    };

    class VulkanDescriptorSet {
    public:
        VulkanDescriptorSet() = default;
        VulkanDescriptorSet(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
        ~VulkanDescriptorSet();

        void setUniformBuffer(VulkanUniformBuffer* uniformBuffer, uint32_t binding, uint32_t index = 0);
        void setTexture(VulkanTexture* texture, uint32_t binding, uint32_t index = 0);

        VkDescriptorSet getHandle() { return set; }
        VkDescriptorSetLayout getLayout() { return layout; }

    private:
        void initBindings();

        VkDescriptorSetLayout layout;
        VkDescriptorSet set;
        std::unordered_map<uint32_t, DescriptorSetBinding> setBindings;
    };

}