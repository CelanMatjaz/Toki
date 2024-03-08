#include "vulkan_render_pass.h"

#include "renderer/vulkan_rendering_context.h"
#include "renderer/vulkan_utils.h"
#include "toki/core/assert.h"

namespace Toki {

VulkanRenderPass::VulkanRenderPass(const RenderPassConfig& config) : m_width(config.width), m_height(config.height) {
    for (uint32_t i = 0; i < config.attachments.size(); ++i) {
        const Attachment& attachment = config.attachments[i];

        VkRenderingAttachmentInfo attachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        attachmentInfo.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
        attachmentInfo.clearValue.depthStencil = { 1.0f, 0 };

        if (attachment.presentable) {
            TK_ASSERT(m_presentableAttachmentIndex == -1, "Only 1 presentable attachment can be provided");
            m_presentableAttachmentIndex = i;
            attachmentInfo.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
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
                m_colorAttachments.emplace_back(createRef<VulkanImage>(VulkanImageConfig{ .format = VulkanUtils::mapFormat(attachment.colorFormat),
                                                                                          .extent = { config.width, config.height, 1 },
                                                                                          .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT }));
                attachmentInfo.imageView = m_colorAttachments.back()->getImageView();
                m_colorAttachmentInfos.emplace_back(attachmentInfo);
                break;
            case ColorFormat::COLOR_FORMAT_DEPTH:
                attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                TK_ASSERT(!m_depthAttachmentInfo, "Depth attachment already provided");
                m_depthAttachment = createRef<VulkanImage>(VulkanImageConfig{ .format = VulkanUtils::mapFormat(attachment.colorFormat),
                                                                              .extent = { config.width, config.height, 1 },
                                                                              .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT });
                attachmentInfo.imageView = m_depthAttachment->getImageView();
                m_depthAttachmentInfo = createRef<VkRenderingAttachmentInfo>(attachmentInfo);
                break;
            case ColorFormat::COLOR_FORMAT_STENCIL:
                TK_ASSERT(!m_stencilAttachmentInfo, "Stencil attachment already provided");
                attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                m_stencilAttachment = createRef<VulkanImage>(VulkanImageConfig{ .format = VulkanUtils::mapFormat(attachment.colorFormat),
                                                                                .extent = { config.width, config.height, 1 },
                                                                                .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT });
                attachmentInfo.imageView = m_stencilAttachment->getImageView();
                m_stencilAttachmentInfo = createRef<VkRenderingAttachmentInfo>(attachmentInfo);
                break;
            case ColorFormat::COLOR_FORMAT_DEPTH_STENCIL:
                TK_ASSERT(!m_depthAttachmentInfo && !m_stencilAttachmentInfo, "Depth/stencil attachment already provided");
                attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                m_depthAttachment = m_stencilAttachment =
                    createRef<VulkanImage>(VulkanImageConfig{ .format = VulkanUtils::mapFormat(attachment.colorFormat),
                                                              .extent = { config.width, config.height, 1 },
                                                              .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT });
                attachmentInfo.imageView = m_depthAttachment->getImageView();
                m_depthAttachmentInfo = m_stencilAttachmentInfo = createRef<VkRenderingAttachmentInfo>(attachmentInfo);

                break;
            default:
                std::unreachable();
        }
    }
}

VulkanRenderPass::~VulkanRenderPass() {}

void VulkanRenderPass::begin(const RenderingContext& ctx, VkExtent2D extent, VkImageView presentImageView) {
    if (presentImageView != VK_NULL_HANDLE && m_presentableAttachmentIndex >= 0) {
        m_colorAttachmentInfos[m_presentableAttachmentIndex].imageView = presentImageView;
    }

    VkRenderingInfoKHR renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.offset = { 0, 0 };
    renderingInfo.renderArea.extent = extent;
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
