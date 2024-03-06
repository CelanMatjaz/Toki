#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

#include "toki/renderer/render_pass.h"

namespace Toki {

class VulkanRenderPass : public RenderPass {
public:
    VulkanRenderPass() = delete;
    VulkanRenderPass(const RenderPassConfig& config);
    ~VulkanRenderPass();

    void begin(const RenderingContext& ctx);
    void end(const RenderingContext& ctx);

private:
    std::vector<VkRenderingAttachmentInfo> m_colorAttachmentInfos;
    Scope<VkRenderingAttachmentInfo> m_depthAttachmentInfo;
    Scope<VkRenderingAttachmentInfo> m_stencilAttachmentInfo;
    int32_t m_presentableAttachmentIndex = -1;
    uint16_t m_width, m_height;
};

}  // namespace Toki
