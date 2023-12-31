#include "vulkan_render_pass.h"

#include "GLFW/glfw3.h"
#include "toki/core/assert.h"
#include "vector"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_utils.h"

namespace Toki {

VulkanRenderPass::VulkanRenderPass(VulkanContext* context, const RenderPassConfig& config) : context(context), config(config) {
    create();
}

VulkanRenderPass::~VulkanRenderPass() {
    destroy();
}

void VulkanRenderPass::create() {
    std::vector<VkAttachmentDescription> attachments(config.attachments.size());
    // TODO: use multiple subpasses
    std::vector<VkSubpassDescription> subpasses;
    std::vector<VkSubpassDependency> dependencies;
    dependencies.reserve(config.attachments.size());

    VkAttachmentReference depthStencilReference{};
    bool hasDepthOrStencilAttachment = false;

    std::vector<VkAttachmentReference> colorAttachmentRefs;

    for (uint32_t i = 0; i < config.attachments.size(); ++i) {
        VkAttachmentReference attachmentRef{};
        attachmentRef.attachment = i;

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = mapFormat(config.attachments[i].format);
        attachmentDescription.samples = mapSamples(config.attachments[i].sampleCount);

        // TODO: refactor
        switch (config.attachments[i].type) {
            case AttachmentType::Color:
                attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                attachmentDescription.loadOp = mapAttachmentLoadOp(config.attachments[i].loadOp);
                attachmentDescription.storeOp = mapAttachmentStoreOp(config.attachments[i].storeOp);
                attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAttachmentRefs.push_back(attachmentRef);
                break;
            case AttachmentType::Depth:
                attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                attachmentDescription.loadOp = mapAttachmentLoadOp(config.attachments[i].loadOp);
                attachmentDescription.storeOp = mapAttachmentStoreOp(config.attachments[i].storeOp);
                attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                hasDepthOrStencilAttachment = true;
                depthStencilReference = attachmentRef;
                break;
            case AttachmentType::Stencil:
                attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                attachmentDescription.stencilLoadOp = mapAttachmentLoadOp(config.attachments[i].loadOp);
                attachmentDescription.stencilStoreOp = mapAttachmentStoreOp(config.attachments[i].storeOp);
                attachmentRef.layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                hasDepthOrStencilAttachment = true;
                depthStencilReference = attachmentRef;
                break;
            case AttachmentType::DepthStencil:
                attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                attachmentDescription.loadOp = mapAttachmentLoadOp(config.attachments[i].loadOp);
                attachmentDescription.storeOp = mapAttachmentStoreOp(config.attachments[i].storeOp);
                attachmentDescription.stencilLoadOp = mapAttachmentLoadOp(config.attachments[i].loadOp);
                attachmentDescription.stencilStoreOp = mapAttachmentStoreOp(config.attachments[i].storeOp);
                attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                hasDepthOrStencilAttachment = true;
                depthStencilReference = attachmentRef;
                break;
            case AttachmentType::None:
                TK_ASSERT(false, "AttachmentType::None provided in render pass config");
        }

        attachments[i] = attachmentDescription;
    }

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = colorAttachmentRefs.size();
    subpass.pColorAttachments = colorAttachmentRefs.data();

    if (hasDepthOrStencilAttachment) {
        subpass.pDepthStencilAttachment = &depthStencilReference;
    }

    subpasses.emplace_back(subpass);

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = attachments.size();
    renderPassCreateInfo.pAttachments = attachments.data();
    renderPassCreateInfo.subpassCount = subpasses.size();
    renderPassCreateInfo.pSubpasses = subpasses.data();
    renderPassCreateInfo.dependencyCount = dependencies.size();
    renderPassCreateInfo.pDependencies = dependencies.data();

    TK_ASSERT_VK_RESULT(
        vkCreateRenderPass(context->device, &renderPassCreateInfo, context->allocationCallbacks, &renderPass), "Could not create render pass"
    );
}

void VulkanRenderPass::destroy() {
    vkDeviceWaitIdle(context->device);

    if (renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(context->device, renderPass, context->allocationCallbacks);
        renderPass = VK_NULL_HANDLE;
    }
}

VkAttachmentDescription VulkanRenderPass::createAttachmentDescription() {
    VkAttachmentDescription attachmentDescription{};

    return attachmentDescription;
}

}  // namespace Toki
