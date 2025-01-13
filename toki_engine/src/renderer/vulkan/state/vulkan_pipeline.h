#pragma once

#include <spirv_cross/spirv_cross.hpp>

#include "renderer/vulkan/state/vulkan_framebuffer.h"
#include "renderer/vulkan/vulkan_types.h"
#include "resources/configs/shader_config_loader.h"

namespace toki {

struct RendererContext;

class VulkanGraphicsPipeline {
public:
    struct Config {
        VulkanFramebuffer& framebuffer;
        configs::ShaderConfig shader_config;
    };

    void create(Ref<RendererContext> ctx, const Config& config);
    void destroy(Ref<RendererContext> ctx);

    VkPipeline get_handle() const;
    VkPipelineLayout get_layout() const;
    VkShaderStageFlags get_push_constant_stage_bits() const;
    const std::vector<VkDescriptorSet>& get_descriptor_sets() const;

    void allocate_descriptor_sets(Ref<RendererContext> ctx, DescriptorPoolManager& pool_manager);

private:
    VkPipeline m_handle;
    PipelineLayout m_layout;
    std::vector<VkDescriptorSet> m_descriptorSets;

private:
    struct PipelineLayoutCreateConfig {
        const DescriptorBindings& bindings;
        const std::vector<VkPushConstantRange>& push_constants;
    };

    static PipelineLayout create_pipeline_layout(Ref<RendererContext> ctx, const PipelineLayoutCreateConfig& config);

    struct PipelineResources {
        PipelineLayout layout;
        std::unordered_map<ShaderStage, VkShaderModule> shader_modules;
    };

    static PipelineResources create_pipeline_resources(Ref<RendererContext> ctx, const std::vector<configs::Shader>& stages);
    static void reflect_shader(ShaderStage stage, std::vector<u32>& binary, DescriptorBindings& bindings, std::vector<VkPushConstantRange>& push_constants);
};

}  // namespace toki
