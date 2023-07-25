#pragma once

#include "vulkan_renderer.h"

namespace Toki {

    class VulkanPipeline {
    public:
        struct PipelineConfig {
            VkPipelineLayout pipelineLayout;
            uint32_t vertShaderIndex;
            uint32_t fragShaderIndex;
            std::vector<VkVertexInputBindingDescription> inputBindingDescriptions;
            std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions;
            bool wireframe = false;
        };

        static VkPipelineLayout createPipelineLayout(
            const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
            const std::vector<VkPushConstantRange>& pushConstants
        );
        static VkPipeline createPipeline(const PipelineConfig& pipelineConfig);

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

        static std::vector<uint32_t> loadShaderCode(const std::filesystem::path& filePath);
        static uint32_t createShaderModule(const std::filesystem::path& filePath);

        static void recreatePipelines();
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
        inline static std::vector<VkDescriptorPool> descriptorPools;
        inline static std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    };

}