#pragma once

#include "renderer/configs.h"
#include "renderer/vulkan/state/vulkan_image.h"

namespace toki {

struct RendererContext;

class VulkanFramebuffer {
public:
    using Config = FramebufferCreateConfig;

    void create(RendererContext* ctx, const Config& config);
    void destroy(RendererContext* ctx);

    void resize(RendererContext* ctx, VkExtent3D extent);

    const std::vector<RenderTarget>& get_render_target_configs() const;
    const std::vector<VkFormat>& get_color_formats() const;
    const std::vector<VulkanImage>& get_images() const;
    const std::optional<VulkanImage>& get_depth_image() const;
    VkFormat get_depth_format() const;
    VkFormat get_stencil_format() const;

    i32 get_present_target_index() const;

    void transition_images(VkCommandBuffer cmd);

private:
    std::vector<RenderTarget> m_initialRenderTargets;
    std::vector<VkFormat> m_colorAttachmentFormats;
    VkFormat m_depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    VkFormat m_stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
    std::vector<VulkanImage> m_images;
    std::optional<VulkanImage> m_depthImage;
    i32 m_presentTargetIndex = -1;
};

}  // namespace toki
