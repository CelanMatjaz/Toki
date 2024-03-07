#include "vulkan_render_pass.h"

#include "renderer/vulkan_rendering_context.h"
#include "toki/core/assert.h"

namespace Toki {

VulkanRenderPass::VulkanRenderPass(const RenderPassConfig& config) : m_width(config.width), m_height(config.height) {
    for (uint32_t i = 0; i < config.attachments.size(); ++i) {
        const Attachment& attachment = config.attachments[i];

        VkRenderingAttachmentInfo attachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        attachmentInfo.clearValue = {};

        if (attachment.presentable) {
            TK_ASSERT(m_presentableAttachmentIndex == -1, "Only 1 presentable attachment can be provided");
            m_presentableAttachmentIndex = i;
        }

        switch (attachment.loadOp) {
            case AttachmentLoadOp::ATTACHMENT_LOAD_OP_CLEAR:
                attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                break;
            case AttachmentLoadOp::ATTACHMENT_LOAD_OP_DONT_CARE:
                attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                break;
            case AttachmentLoadOp::ATTACHMENT_LOAD_OP_LOAD:
                attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                break;
            default:
                std::unreachable();
        }

        switch (attachment.storeOp) {
            case AttachmentStoreOp::ATTACHMENT_STORE_OP_STORE:
                attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                break;
            case AttachmentStoreOp::ATTACHMENT_STORE_OP_DONT_CARE:
                attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                break;
            default:
                std::unreachable();
        }

        switch (attachment.colorFormat) {
            case ColorFormat::COLOR_FORMAT_R:
            case ColorFormat::COLOR_FORMAT_RG:
            case ColorFormat::COLOR_FORMAT_RGB:
            case ColorFormat::COLOR_FORMAT_RGBA:
                attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                m_colorAttachmentInfos.emplace_back(attachmentInfo);
                break;
            case ColorFormat::COLOR_FORMAT_DEPTH:
                TK_ASSERT(!m_depthAttachmentInfo, "Depth attachment already provided");
                m_depthAttachmentInfo = createScope<VkRenderingAttachmentInfo>(attachmentInfo);
                break;
            case ColorFormat::COLOR_FORMAT_STENCIL:
                TK_ASSERT(!m_stencilAttachmentInfo, "Stencil attachment already provided");
                m_stencilAttachmentInfo = createScope<VkRenderingAttachmentInfo>(attachmentInfo);
                break;
            case ColorFormat::COLOR_FORMAT_DEPTH_STENCIL:
                TK_ASSERT(!m_depthAttachmentInfo && !m_stencilAttachmentInfo, "Depth/stencil attachment already provided");
                m_depthAttachmentInfo = createScope<VkRenderingAttachmentInfo>(attachmentInfo);
                m_stencilAttachmentInfo = createScope<VkRenderingAttachmentInfo>(attachmentInfo);
            default:
                std::unreachable();
        }
    }
}

VulkanRenderPass::~VulkanRenderPass() {}

void VulkanRenderPass::begin(const RenderingContext& ctx, VkImageView presentImageView) {
    if (presentImageView != VK_NULL_HANDLE && m_presentableAttachmentIndex >= 0) {
        m_colorAttachmentInfos[m_presentableAttachmentIndex].imageView = presentImageView;
    }

    VkRenderingInfoKHR renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.offset = { 0, 0 };
    renderingInfo.renderArea.extent = { 800, 600 };
    renderingInfo.layerCount = 1;
    renderingInfo.viewMask = 0;
    renderingInfo.colorAttachmentCount = m_colorAttachmentInfos.size();
    renderingInfo.pColorAttachments = m_colorAttachmentInfos.data();
    renderingInfo.pDepthAttachment = m_depthAttachmentInfo.get();
    renderingInfo.pStencilAttachment = m_stencilAttachmentInfo.get();

    vkCmdBeginRendering(((VulkanRenderingContext&) ctx).getCommandBuffer(), &renderingInfo);
}

void VulkanRenderPass::end(const RenderingContext& ctx) {
    vkCmdEndRendering(((VulkanRenderingContext&) ctx).getCommandBuffer());
}

}  // namespace Toki
