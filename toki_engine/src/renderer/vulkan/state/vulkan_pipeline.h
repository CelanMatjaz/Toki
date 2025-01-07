#pragma once

#include "renderer/vulkan/state/vulkan_framebuffer.h"

namespace toki {

struct RendererContext;

class VulkanGraphicsPipeline {
public:
    struct Config {
        VulkanFramebuffer& framebuffer;
        std::filesystem::path vertex_shader_path;
        std::filesystem::path fragment_shader_path;
    };

    void create(Ref<RendererContext> ctx, const Config& config);
    void destroy(Ref<RendererContext> ctx);

    VkPipeline get_handle() const;
    VkPipelineLayout get_layout() const;

private:
    VkPipeline m_handle;
    VkPipelineLayout m_layout;
};

}  // namespace toki
