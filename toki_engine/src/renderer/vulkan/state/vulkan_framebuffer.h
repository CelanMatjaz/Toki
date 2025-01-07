#pragma once

#include "renderer/vulkan/state/vulkan_image.h"
#include "renderer/vulkan/vulkan_types.h"

namespace toki {

struct RendererContext;

class VulkanFramebuffer {
public:
    struct Config {
        std::vector<RenderTarget> render_targets;
    };

    void create(Ref<RendererContext> ctx, const Config& config);
    void destroy(Ref<RendererContext> ctx);

    void resize(Ref<RendererContext> ctx, VkExtent3D extent);

    const std::vector<VkRenderingAttachmentInfo>& get_color_attachments_rendering_infos() const;
    const VkPipelineRenderingCreateInfoKHR get_pipeline_rendering_create_info() const;

    i32 get_present_target_index() const;

private:
    RenderTarget m_initialRenderTargets[MAX_FRAMEBUFFER_ATTACHMENTS];
    std::vector<VkRenderingAttachmentInfo> m_colorAttachmentInfos;
    std::vector<VkFormat> m_attachmentFormats;
    VkPipelineRenderingCreateInfoKHR m_pipelineRenderingCreateInfo;
    std::vector<VulkanImage> m_images;
    i32 m_presentTargetIndex = -1;
};

}  // namespace toki
