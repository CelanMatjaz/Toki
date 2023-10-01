#include "tkpch.h"
#include "vulkan_framebuffer.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "platform/vulkan/vulkan_texture.h"
#include "platform/vulkan/backend/vulkan_utils.h"
#include "core/core.h"
#include "core/assert.h"

namespace Toki {

    VulkanFramebuffer::VulkanFramebuffer(const FramebufferConfig& config) : Framebuffer(config) {
        create();
    }

    VulkanFramebuffer::~VulkanFramebuffer() {
        destroy();
    }

    void VulkanFramebuffer::bind() {
        VkCommandBuffer cmd = VulkanRenderer::commandBuffer();

        std::vector<Attachment> attachments = ((VulkanRenderPass*) config.renderPass.get())->getAttachments();
        uint32_t nAttachments = attachments.size();

        VkClearValue clear = { { config.clearColor.r, config.clearColor.g, config.clearColor.b, config.clearColor.a } };
        std::vector<VkClearValue> clearValues(nAttachments, clear);

        for (uint32_t i = 0; i < nAttachments; ++i) {
            if (attachments[i].isDepthAttachment)
                clearValues[i].depthStencil = { 1.0f, 0 };
        }

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = ((VulkanRenderPass*) config.renderPass.get())->getHandle();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = VulkanRenderer::swapchain()->getExtent();
        renderPassBeginInfo.framebuffer = framebuffers[VulkanRenderer::currentFrameIndex()];
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanFramebuffer::resize(uint32_t width, uint32_t height, uint32_t layers) {
        config.width = width;
        config.height = height;
        destroy();
        create();
    }

    void VulkanFramebuffer::nextSubpass() {
        vkCmdNextSubpass(VulkanRenderer::commandBuffer(), VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanFramebuffer::unbind() {
        vkCmdEndRenderPass(VulkanRenderer::commandBuffer());
    }

    float VulkanFramebuffer::readPixel(uint32_t attachmentIndex, uint32_t x, uint32_t y, uint32_t z) {
        return attachments[VulkanRenderer::currentFrameIndex()][attachmentIndex]->readPixel(x, y);
    }

    Ref<Texture> VulkanFramebuffer::getAttachment(uint32_t attachmentIndex) {
        TK_ASSERT(attachmentIndex < attachments[VulkanRenderer::currentFrameIndex()].size(), std::format("Attachment index {} out of bounds", attachmentIndex));
        return createRef<VulkanTexture>(attachments[VulkanRenderer::currentFrameIndex()][attachmentIndex]);
    }

    void VulkanFramebuffer::create() {
        std::vector<Attachment> renderPassAttachments = ((VulkanRenderPass*) config.renderPass.get())->getAttachments();

        for (uint32_t i = 0; i < VulkanContext::MAX_FRAMES; ++i) {
            attachments[i].resize(renderPassAttachments.size());
        }

        for (uint32_t i = 0; i < renderPassAttachments.size(); ++i) {
            if (renderPassAttachments[i].target == RenderTarget::Swapchain) {
                for (uint32_t index = 0; index < VulkanContext::MAX_FRAMES; ++index) {
                    attachments[index][i] = VulkanRenderer::swapchain()->images[index];
                }
            }
            else if (renderPassAttachments[i].isDepthAttachment) {
                VulkanImageConfig vulkanImageConfig{};
                vulkanImageConfig.extent = { config.width, config.height, 1 };
                vulkanImageConfig.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                vulkanImageConfig.format = VulkanUtils::mapFormat(renderPassAttachments[i].format);
                attachments[0][i] = createRef<VulkanImage>(vulkanImageConfig);
            }
            else {
                VulkanImageConfig vulkanImageConfig{};
                vulkanImageConfig.extent = { config.width, config.height, 1 };
                vulkanImageConfig.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                vulkanImageConfig.format = VulkanUtils::mapFormat(renderPassAttachments[i].format);
                attachments[0][i] = createRef<VulkanImage>(vulkanImageConfig);
            }
        }

        for (uint32_t index = 0; index < VulkanContext::MAX_FRAMES; ++index) {
            std::vector<VkImageView> imageViews(renderPassAttachments.size());

            for (uint32_t i = 0; i < renderPassAttachments.size(); ++i) {

                if (renderPassAttachments[i].target == RenderTarget::Swapchain)
                    imageViews[i] = attachments[index][i]->getView();
                else
                    imageViews[i] = attachments[0][i]->getView();

            }

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = ((VulkanRenderPass*) config.renderPass.get())->getHandle();
            framebufferInfo.width = config.width;
            framebufferInfo.height = config.height;
            framebufferInfo.layers = 1;
            framebufferInfo.attachmentCount = imageViews.size();
            framebufferInfo.pAttachments = imageViews.data();

            TK_ASSERT_VK_RESULT(vkCreateFramebuffer(VulkanRenderer::device(), &framebufferInfo, nullptr, &framebuffers[index]), "Could not create framebuffer");
        }
    }

    void VulkanFramebuffer::destroy() {
        vkDeviceWaitIdle(VulkanRenderer::device());
        if (config.target == RenderTarget::Texture) {
            vkDestroyFramebuffer(VulkanRenderer::device(), framebuffers[0], nullptr);
        }
        else {
            for (uint32_t i = 0; i < VulkanContext::MAX_FRAMES; ++i) {
                vkDestroyFramebuffer(VulkanRenderer::device(), framebuffers[i], nullptr);
            }
        }

        attachments->clear();
    }

}