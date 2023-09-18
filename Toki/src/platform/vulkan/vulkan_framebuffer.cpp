#include "tkpch.h"
#include "vulkan_framebuffer.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "platform/vulkan/backend/vulkan_utils.h"
#include "core/core.h"
#include "core/assert.h"

namespace Toki {

    VulkanFramebuffer::VulkanFramebuffer(const FramebufferConfig& config) : Framebuffer(config) {
        TK_ASSERT(config.target != RenderTarget::None, "RenderTarget::None when creating a framebuffer is not supported");
        create();
    }

    VulkanFramebuffer::~VulkanFramebuffer() {
        destroy();
    }

    void VulkanFramebuffer::bind() {
        VkCommandBuffer cmd = VulkanRenderer::commandBuffer();

        bool hasDepthAttachment = config.depthAttachment.get() != nullptr;
        uint32_t nAttachments = hasDepthAttachment ? config.colorAttachments.size() + 1 : config.colorAttachments.size();

        VkClearValue clear = { { config.clearColor.r, config.clearColor.g, config.clearColor.b, config.clearColor.a } };
        std::vector<VkClearValue> clearValues(nAttachments, clear);

        if (hasDepthAttachment) {
            clearValues.back().depthStencil = { 1.f, 0 };
        }

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderPass->getHandle();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = VulkanRenderer::swapchain()->getExtent();
        renderPassBeginInfo.framebuffer = framebuffers[VulkanRenderer::currentFrameIndex()];
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanFramebuffer::unbind() {
        vkCmdEndRenderPass(VulkanRenderer::commandBuffer());
    }

    float VulkanFramebuffer::readPixel(uint32_t attachmentIndex, uint32_t x, uint32_t y, uint32_t z) {
        return attachments[VulkanRenderer::currentFrameIndex()][attachmentIndex]->readPixel(x, y);
    }

    void VulkanFramebuffer::create() {
        VulkanRenderPassConfig vulkanRenderPassConfig;
        vulkanRenderPassConfig.colorAttachments = config.colorAttachments;
        vulkanRenderPassConfig.depthAttachment = config.depthAttachment;
        renderPass = createRef<VulkanRenderPass>(vulkanRenderPassConfig);

        bool hasDepthAttachment = config.depthAttachment.get() != nullptr;
        uint32_t nAttachments = hasDepthAttachment ? config.colorAttachments.size() + 1 : config.colorAttachments.size();

        std::vector<Ref<VulkanImage>> textureAttachments(config.colorAttachments.size());

        for (uint32_t i = 0; i < config.colorAttachments.size(); ++i) {
            if (config.colorAttachments[i].target == RenderTarget::Swapchain) continue;
            VulkanImageConfig vulkanImageConfig{};
            vulkanImageConfig.extent = { config.width, config.height, 1 };
            vulkanImageConfig.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
            vulkanImageConfig.format = VulkanUtils::mapFormat(config.colorAttachments[i].format);
            textureAttachments[i] = createRef<VulkanImage>(vulkanImageConfig);
        }

        auto createFramebuffer = [nAttachments, config = this->config, hasDepthAttachment, textureAttachments](std::vector<Ref<VulkanImage>>& attachments, VkFramebuffer* framebuffer, VkRenderPass* renderPass, uint32_t index = 0) {
            std::vector<VkImageView> imageViews(nAttachments);
            uint32_t swapchainTargetAttachmentCount = 0;

            VulkanImageConfig vulkanImageConfig{};
            vulkanImageConfig.extent = { config.width, config.height, 1 };
            vulkanImageConfig.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = *renderPass;
            framebufferInfo.width = config.width;
            framebufferInfo.height = config.height;
            framebufferInfo.layers = 1;

            for (uint32_t i = 0; i < config.colorAttachments.size(); ++i) {
                if (config.colorAttachments[i].target == RenderTarget::Swapchain) {
                    attachments.emplace_back(VulkanRenderer::swapchain()->images[index]);
                    imageViews[i] = VulkanRenderer::swapchain()->images[index]->getView();
                    TK_ASSERT(++swapchainTargetAttachmentCount <= 1, "Framebuffer is limited to only 1 swapchain attachment texture");
                }
                else {
                    attachments.emplace_back(textureAttachments[i]);
                    imageViews[i] = attachments[i]->getView();
                }
            }

            if (hasDepthAttachment) {
                vulkanImageConfig.format = VulkanUtils::mapFormat(config.depthAttachment->format);
                vulkanImageConfig.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                attachments.emplace_back(createRef<VulkanImage>(vulkanImageConfig));
                imageViews.back() = attachments.back()->getView();
            }

            framebufferInfo.attachmentCount = imageViews.size();
            framebufferInfo.pAttachments = imageViews.data();

            TK_ASSERT_VK_RESULT(vkCreateFramebuffer(VulkanRenderer::device(), &framebufferInfo, nullptr, framebuffer), "Could not create framebuffer");
        };

        for (uint32_t i = 0; i < VulkanContext::MAX_FRAMES; ++i) {
            VkRenderPass renderPass = this->renderPass->getHandle();
            createFramebuffer(attachments[i], &framebuffers[i], &renderPass, i);
            if (config.target == RenderTarget::Texture) break;
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
    }

}