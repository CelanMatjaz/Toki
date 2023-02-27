#pragma once

#include "vulkan_renderer.h"

namespace Toki {

    class VulkanPipeline {
    public:
        struct PipelineConfig {
            vk::PipelineLayout pipelineLayout;
            vk::ShaderModule vertShaderModule;
            vk::ShaderModule fragShaderModule;
            std::vector<vk::VertexInputBindingDescription> inputBindingDescriptions;
            std::vector<vk::VertexInputAttributeDescription> inputAttributeDescriptions;
        };

        static vk::PipelineLayout createPipelineLayout(
            const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
            const std::vector<vk::PushConstantRange>& pushConstants
        );
        static vk::Pipeline createPipeline(const PipelineConfig& pipelineConfig);

        struct LayoutData {
            uint32_t binding;
            vk::DescriptorType descriptorType;
            uint32_t count;
            vk::ShaderStageFlagBits stageFlags;
        };

        using DescriptorSetLayoutData = std::vector<LayoutData>;

        static vk::DescriptorSetLayout createDescriptorSetLayout(const DescriptorSetLayoutData& bindings);
        static vk::DescriptorPool createDescriptorPool(vk::DescriptorType type, uint32_t size);
        static std::vector<vk::DescriptorSet> createDescriptorSets(vk::DescriptorPool descriptorPool, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts);
    };

}