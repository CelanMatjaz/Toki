#include "tkpch.h"
#include "vulkan_framebuffer.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "platform/vulkan/backend/vulkan_utils.h"
#include "core/core.h"
#include "core/assert.h"

namespace Toki {

    VulkanFramebuffer::VulkanFramebuffer(const FramebufferConfig& config) : Framebuffer(config) {
        TK_ASSERT(config.target != RenderTarget::None, "RenderTarget::None when creating a framebuffer is not supported");

        VulkanRenderPassConfig vulkanRenderPassConfig;
        vulkanRenderPassConfig.colorAttachments = config.colorAttachments;
        vulkanRenderPassConfig.depthAttachment = config.depthAttachment;
        renderPass = createRef<VulkanRenderPass>(vulkanRenderPassConfig);

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
        renderPassBeginInfo.renderArea.extent = { config.width, config.height };
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        if (isSwapchainTarget) {
            renderPassBeginInfo.framebuffer = framebuffers[VulkanRenderer::currentFrameIndex()];
        }
        else {
            renderPassBeginInfo.framebuffer = framebuffers[0];
        }

        vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanFramebuffer::unbind() {
        vkCmdEndRenderPass(VulkanRenderer::commandBuffer());
    }

    void VulkanFramebuffer::resize(uint32_t width, uint32_t height, uint32_t layers) {
        config.width = width;
        config.height = height;
        config.layers = layers;
        destroy();
        create();
    }

    void VulkanFramebuffer::create() {
        bool hasDepthAttachment = config.depthAttachment.get() != nullptr;
        uint32_t nAttachments = hasDepthAttachment ? config.colorAttachments.size() + 1 : config.colorAttachments.size();

        auto createFramebuffer = [nAttachments, config = this->config, hasDepthAttachment](std::vector<Ref<VulkanImage>>& attachments, VkFramebuffer* framebuffer, VkRenderPass* renderPass, uint32_t index = 0) {
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
            framebufferInfo.layers = config.layers;

            for (uint32_t i = 0; i < config.colorAttachments.size(); ++i) {
                if (config.colorAttachments[i].target == RenderTarget::Swapchain) {
                    attachments.emplace_back(VulkanRenderer::swapchain()->images[index]);

                    framebufferInfo.width = VulkanRenderer::swapchain()->getExtent().width;
                    framebufferInfo.height = VulkanRenderer::swapchain()->getExtent().height;

                    imageViews[i] = VulkanRenderer::swapchain()->images[index]->getView();
                    TK_ASSERT(++swapchainTargetAttachmentCount <= 1, "Framebuffer is limited to only 1 swapchain attachment texture");
                }
                else {
                    vulkanImageConfig.format = VulkanUtils::mapFormat(config.colorAttachments[i].format);
                    attachments.emplace_back(createRef<VulkanImage>(vulkanImageConfig));
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

            return (bool) swapchainTargetAttachmentCount;
        };

        isSwapchainTarget = false;

        for (uint32_t i = 0; i < VulkanContext::MAX_FRAMES; ++i) {
            VkRenderPass renderPass = this->renderPass->getHandle();
            attachments[i].clear();
            isSwapchainTarget = createFramebuffer(attachments[i], &framebuffers[i], &renderPass, i);
            if (config.target == RenderTarget::Texture) break;
        }

        int a = 0;
    }

    void VulkanFramebuffer::destroy() {
        vkDeviceWaitIdle(VulkanRenderer::device());

        attachments->clear();

        if (config.target == RenderTarget::Texture) {
            vkDestroyFramebuffer(VulkanRenderer::device(), framebuffers[0], nullptr);
            framebuffers[0] = VK_NULL_HANDLE;
            return;
        }

        for (uint32_t i = 0; i < VulkanContext::MAX_FRAMES; ++i) {
            vkDestroyFramebuffer(VulkanRenderer::device(), framebuffers[i], nullptr);
            framebuffers[i] = VK_NULL_HANDLE;
        }
    }

}