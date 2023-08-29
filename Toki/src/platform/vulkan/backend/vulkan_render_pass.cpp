#include "tkpch.h"
#include "vulkan_render_pass.h"
#include "core/core.h"
#include "core/assert.h"
#include "platform/vulkan/backend/vulkan_utils.h"
#include "platform/vulkan/vulkan_renderer.h"

namespace Toki {

    VulkanRenderPass::VulkanRenderPass(VulkanRenderPassConfig& config) {
        std::vector<VkAttachmentDescription> attachments(config.colorAttachments.size() + 1);
        std::vector<VkAttachmentReference> attachmentRefs(config.colorAttachments.size());

        uint32_t swapchainTargets = 0;

        for (uint32_t i = 0; i < config.colorAttachments.size(); ++i) {
            bool isSwapchainTarget = config.colorAttachments[i].target == RenderTarget::Swapchain;
            attachments[i] = createColorAttachmentDescription(config.colorAttachments[i], config.colorAttachments[i].target == RenderTarget::Swapchain);
            attachmentRefs[i].attachment = i;
            attachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            if (isSwapchainTarget) ++swapchainTargets;
        }

        TK_ASSERT(swapchainTargets <= 1, "Only 1 RenderTarget::Swapchain attachment can exist on a framebuffer");

        bool hasDepthAttachment = config.depthAttachment.get() != nullptr;
        uint32_t attachmentCount = hasDepthAttachment ? config.colorAttachments.size() + 1 : config.colorAttachments.size();
        VkAttachmentDescription depthAttachment{};
        VkAttachmentReference depthAttachmentRef{};
        if (hasDepthAttachment) {
            attachments.back() = createDepthAttachmentDescription(*config.depthAttachment.get());
            depthAttachmentRef.attachment = attachments.size() - 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = attachmentRefs.size();
        subpass.pColorAttachments = attachmentRefs.data();

        if (hasDepthAttachment) {
            subpass.pDepthStencilAttachment = &depthAttachmentRef;
        }

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        if (hasDepthAttachment) {
            dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachmentCount;
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &dependency;

        TK_ASSERT_VK_RESULT(vkCreateRenderPass(VulkanRenderer::device(), &renderPassCreateInfo, nullptr, &handle), "Could not create render pass");
    }

    VulkanRenderPass::~VulkanRenderPass() {
        vkDeviceWaitIdle(VulkanRenderer::device());
        vkDestroyRenderPass(VulkanRenderer::device(), handle, nullptr);
    }

    VkAttachmentDescription VulkanRenderPass::createColorAttachmentDescription(const Attachment& attachment, bool isSwapchainAttachment) {
        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = isSwapchainAttachment ? VulkanRenderer::swapchain()->getFormat() : VulkanUtils::mapFormat(attachment.format);
        attachmentDescription.samples = VulkanUtils::mapSamples(attachment.samples);
        attachmentDescription.loadOp = VulkanUtils::mapLoadOp(attachment.loadOp);
        attachmentDescription.storeOp = VulkanUtils::mapStoreOp(attachment.storeOp);
        attachmentDescription.stencilLoadOp = VulkanUtils::mapLoadOp(attachment.loadOp);
        attachmentDescription.stencilStoreOp = VulkanUtils::mapStoreOp(attachment.storeOp);
        attachmentDescription.initialLayout = (attachment.initialLayout == InitialLayout::Undefined ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR); // TODO: create mapping functioon
        attachmentDescription.finalLayout = isSwapchainAttachment ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        return attachmentDescription;
    }

    VkAttachmentDescription VulkanRenderPass::createDepthAttachmentDescription(const Attachment& attachment) {
        VkAttachmentDescription attachmentDescription = createColorAttachmentDescription(attachment);
        attachmentDescription.format = VulkanUtils::findDepthFormat();
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        return attachmentDescription;
    }

}