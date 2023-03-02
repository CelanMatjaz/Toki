#pragma once

#include "vulkan_renderer.h"

namespace Toki {

    class VulkanPipeline {
    public:
        struct PipelineConfig {
            uint32_t pipelineLayoutIndex;
            uint32_t vertShaderIndex;
            uint32_t fragShaderIndex;
            std::vector<vk::VertexInputBindingDescription> inputBindingDescriptions;
            std::vector<vk::VertexInputAttributeDescription> inputAttributeDescriptions;
        };

        static uint32_t createPipelineLayout(
            const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
            const std::vector<vk::PushConstantRange>& pushConstants
        );
        static uint32_t createPipeline(const PipelineConfig& pipelineConfig);

        struct LayoutData {
            uint32_t binding;
            vk::DescriptorType descriptorType;
            uint32_t count;
            vk::ShaderStageFlagBits stageFlags;
        };

        struct DestriptorPoolSize {
            vk::DescriptorType type;
            uint32_t size;
        };

        using DescriptorSetLayoutData = std::vector<LayoutData>;
        using DestriptorPoolSizes = std::vector<DestriptorPoolSize>;

        static vk::DescriptorSetLayout createDescriptorSetLayout(const DescriptorSetLayoutData& bindings);
        static vk::DescriptorPool createDescriptorPool(const DestriptorPoolSizes& sizes);
        static std::vector<vk::DescriptorSet> createDescriptorSets(vk::DescriptorPool descriptorPool, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts);

        static std::vector<char> loadShaderCode(const std::filesystem::path& filePath);
        static uint32_t createShaderModule(const std::filesystem::path& filePath);

        static void recreatePipelines();
        static const vk::Pipeline getPipeline(uint32_t index);
        static const vk::PipelineLayout getPipelineLayout(uint32_t index);
        static void cleanup();

    private:
        static vk::Pipeline _createPipeline(const PipelineConfig& pipelineConfig);

        struct PipelineData {
            vk::Pipeline pipeline;
            PipelineConfig config;
        };

        inline static std::vector<PipelineData> pipelines;
        inline static std::vector<vk::PipelineLayout> pipelineLayouts;
        inline static std::vector<vk::ShaderModule> shaderModules;
    };

}