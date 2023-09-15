#pragma once

#include "core/core.h"
#include "vulkan/vulkan.h"
#include "renderer/shader.h"
#include "platform/vulkan/backend/vulkan_pipeline.h"
#include "platform/vulkan/backend/vulkan_render_pass.h"
#include "unordered_map"

namespace Toki {

    struct DestriptorPoolSize {
        VkDescriptorType type;
        uint32_t size;
    };

    using DestriptorPoolSizes = std::vector<DestriptorPoolSize>;

    struct PipelineOptions {
        bool wireframe = false;
    };

    struct PipelineConfig {
        ShaderType type = ShaderType::None;
        VkPipelineLayout layout;
        std::unordered_map<ShaderStage, VkShaderModule> moduleSpirv;
        std::vector<VkVertexInputBindingDescription> inputBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions;
        PipelineProperties properties{};
        Ref<VulkanRenderPass> renderPass;
    };

    class Pipeline {
    public:
        static Ref<Pipeline> create(const PipelineConfig& config);

        Pipeline(const PipelineConfig& config);
        virtual ~Pipeline() = default;

        virtual void create() = 0;
        virtual void destroy() = 0;

        VkPipeline getHandle() { return pipeline; }

    protected:
        PipelineConfig config;
        VkPipeline pipeline = VK_NULL_HANDLE;
    };

    class GraphicsPipeline : public Pipeline {
    public:
        GraphicsPipeline(const PipelineConfig& config);
        ~GraphicsPipeline();

        virtual void create() override;
        virtual void destroy() override;

    private:
    };

}