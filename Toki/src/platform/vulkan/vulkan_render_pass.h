#pragma once

#include "renderer/render_pass.h"
#include "vulkan/vulkan.h"

namespace Toki {

    class VulkanRenderPass : public RenderPass {
    public:
        VulkanRenderPass(const RenderPassConfig& config);
        ~VulkanRenderPass();

        VkRenderPass getHandle() const { return renderPassHandle; }
        uint8_t getColorAttachmentCount(uint32_t subpassIndex) const { return colorAttachmentCounts[subpassIndex]; }
        bool hasDepthAttachment() const { return hasDepthAttachment_; }
        const std::vector<Attachment>& getAttachments() const { return config.attachments; }

    private:
        static VkAttachmentDescription createColorAttachmentDescription(const Attachment& attachment, bool isSwapchainAttachment = false);
        static VkAttachmentDescription createDepthAttachmentDescription(const Attachment& attachment);
        static VkSubpassDependency createColorDependency(uint32_t srcSubpass, uint32_t dstSubpass);
        static VkSubpassDependency createDepthDependency(uint32_t srcSubpass, uint32_t dstSubpass);

        VkRenderPass renderPassHandle = VK_NULL_HANDLE;
        std::vector<uint8_t> colorAttachmentCounts;
        bool hasDepthAttachment_ = false;
    };

}