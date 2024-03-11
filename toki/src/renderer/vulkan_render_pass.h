#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

#include "renderer/vulkan_image.h"
#include "toki/renderer/render_pass.h"

namespace Toki {
class VulkanRenderer;

class VulkanRenderPass : public RenderPass {
    friend VulkanRenderer;

public:
    VulkanRenderPass() = delete;
    VulkanRenderPass(const RenderPassConfig& config);
    ~VulkanRenderPass();

    virtual Ref<Texture> getColorAttachment(uint32_t textureIndex) override;
    virtual Ref<Texture> getDepthAttachment() override;
    virtual Ref<Texture> getStencilAttachment() override;

    void begin(const RenderingContext& ctx, VkExtent2D extent, VkImageView presentImageView = VK_NULL_HANDLE);
    void end(const RenderingContext& ctx);

protected:
    inline static Ref<VulkanContext> s_context;

    std::vector<VkRenderingAttachmentInfo> m_colorAttachmentInfos;
    std::vector<Ref<VulkanImage>> m_colorAttachments;
    Ref<VkRenderingAttachmentInfo> m_depthAttachmentInfo;
    Ref<VulkanImage> m_depthAttachment;
    Ref<VkRenderingAttachmentInfo> m_stencilAttachmentInfo;
    Ref<VulkanImage> m_stencilAttachment;
    int32_t m_presentableAttachmentIndex = -1;
    uint16_t m_width, m_height;
};

}  // namespace Toki
