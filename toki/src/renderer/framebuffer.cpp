#include "framebuffer.h"

#include "renderer/renderer_state.h"
#include "renderer/vulkan_types.h"

namespace Toki {

Framebuffer::Framebuffer(const std::vector<Attachment>& attachments, const Extent3D& extent, Handle swapchainHandle) :
    m_attachments(attachments),
    m_renderPassHandle(attachments),
    m_swapchainHandle(swapchainHandle) {
    if (!s_renderPassMap.contains(m_renderPassHandle)) {
        s_renderPassMap.emplace(m_renderPassHandle, createRef<RenderPass>(attachments));
    }

    m_attachmentImages.resize(attachments.size(), nullptr);

    bool isPresentable = false;
    for (uint32_t i = 0; const auto& attachment : m_attachments) {
        if (!attachment.presentable) {
            m_attachmentImages[i] = createRef<Texture>(attachment.colorFormat, extent.x, extent.y, extent.z);
        } else {
            isPresentable = true;
        }

        ++i;
    }

    if (m_swapchainHandle && isPresentable) {
        m_framebuffers.resize(MAX_FRAMES, VK_NULL_HANDLE);
        auto extent = s_swapchainMap[swapchainHandle]->m_extent;
        recreate({ extent.width, extent.height, 1.0f });
    } else {
        recreate(extent);
    }
}

Framebuffer::~Framebuffer() {
    destroy();
}

void Framebuffer::destroy() {
    for (auto& framebuffer : m_framebuffers) {
        if (framebuffer == VK_NULL_HANDLE) {
            continue;
        }
        vkDestroyFramebuffer(context.device, framebuffer, context.allocationCallbacks);
    }
}

void Framebuffer::recreate(const Extent3D& extent) {
    destroy();

    m_extent = { (uint32_t) extent.x, (uint32_t) extent.y, (uint32_t) extent.z };

    std::vector<VkImageView> imageViews(m_attachments.size());
    int32_t presentableAttachmentIndex = -1;
    for (uint32_t i = 0; i < m_attachments.size(); ++i) {
        if (!m_attachments[i].presentable) {
            m_attachmentImages[i]->resize(extent);
            imageViews[i] = m_attachmentImages[i]->m_imageView;
            continue;
        } else {
            presentableAttachmentIndex = i;
        }
    }

    VkFramebufferCreateInfo framebufferCreateInfo{};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = *s_renderPassMap[m_renderPassHandle];
    framebufferCreateInfo.width = extent.x;
    framebufferCreateInfo.height = extent.y;
    framebufferCreateInfo.layers = extent.z;

    if (presentableAttachmentIndex == -1) {
        framebufferCreateInfo.attachmentCount = imageViews.size();
        framebufferCreateInfo.pAttachments = imageViews.data();
        TK_ASSERT_VK_RESULT(
            vkCreateFramebuffer(context.device, &framebufferCreateInfo, context.allocationCallbacks, &m_framebuffers[0]),
            "Could not create framebuffer");
        return;
    }

    TK_ASSERT(m_swapchainHandle, "Swapchain handle for framebuffer is invalid");
    auto& swapchain = s_swapchainMap[m_swapchainHandle];

    m_framebuffers.resize(MAX_FRAMES);

    for (uint32_t i = 0; i < MAX_FRAMES; ++i) {
        imageViews[presentableAttachmentIndex] = swapchain->m_wrappedImages[i]->m_imageView;
        framebufferCreateInfo.attachmentCount = imageViews.size();
        framebufferCreateInfo.pAttachments = imageViews.data();

        TK_ASSERT_VK_RESULT(
            vkCreateFramebuffer(context.device, &framebufferCreateInfo, context.allocationCallbacks, &m_framebuffers[i]),
            "Could not create framebuffer");
    }
}

Framebuffer::operator VkFramebuffer() {
    if (m_swapchainHandle) {
        return m_framebuffers[s_swapchainMap[m_swapchainHandle]->m_imageIndex];
    } else {
        return m_framebuffers[0];
    }
}

}  // namespace Toki
