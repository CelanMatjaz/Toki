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
    };

}