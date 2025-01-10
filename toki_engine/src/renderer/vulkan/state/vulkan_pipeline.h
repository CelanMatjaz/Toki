#pragma once

#include "renderer/vulkan/state/vulkan_framebuffer.h"
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

private:
    VkPipeline m_handle;
    VkPipelineLayout m_layout;

    static VkShaderModule create_shader_module(Ref<RendererContext> ctx, configs::Shader shader);
};

}  // namespace toki
