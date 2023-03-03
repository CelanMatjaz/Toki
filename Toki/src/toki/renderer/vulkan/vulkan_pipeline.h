#pragma once

#include "vulkan_renderer.h"

namespace Toki {

    class VulkanPipeline {
    public:
        struct PipelineConfig {
            uint32_t pipelineLayoutIndex;
            uint32_t vertShaderIndex;
            uint32_t fragShaderIndex;
            std::vector<VkVertexInputBindingDescription> inputBindingDescriptions;
            std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions;
        };

        static uint32_t createPipelineLayout(
            const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
            const std::vector<VkPushConstantRange>& pushConstants
        );
        static uint32_t createPipeline(const PipelineConfig& pipelineConfig);

        struct LayoutData {
            uint32_t binding;
            VkDescriptorType descriptorType;
            uint32_t count;
            VkShaderStageFlagBits stageFlags;
        };

        struct DestriptorPoolSize {
            VkDescriptorType type;
            uint32_t size;
        };

        using DescriptorSetLayoutData = std::vector<LayoutData>;
        using DestriptorPoolSizes = std::vector<DestriptorPoolSize>;

        static VkDescriptorSetLayout createDescriptorSetLayout(const DescriptorSetLayoutData& bindings);
        static VkDescriptorPool createDescriptorPool(const DestriptorPoolSizes& sizes);
        static std::vector<VkDescriptorSet> createDescriptorSets(VkDescriptorPool descriptorPool, std::vector<VkDescriptorSetLayout> descriptorSetLayouts);

        static std::vector<char> loadShaderCode(const std::filesystem::path& filePath);
        static uint32_t createShaderModule(const std::filesystem::path& filePath);

        static void recreatePipelines();
        static const VkPipeline getPipeline(uint32_t index);
        static const VkPipelineLayout getPipelineLayout(uint32_t index);
        static void cleanup();

    private:
        static VkPipeline _createPipeline(const PipelineConfig& pipelineConfig);

        struct PipelineData {
            VkPipeline pipeline;
            PipelineConfig config;
        };

        inline static std::vector<PipelineData> pipelines;
        inline static std::vector<VkPipelineLayout> pipelineLayouts;
        inline static std::vector<VkShaderModule> shaderModules;
    };

}