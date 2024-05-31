#include "render_pass.h"

#include <optional>

#include "renderer/mapping_functions.h"
#include "renderer/vulkan_types.h"
#include "toki/core/assert.h"

namespace Toki {

RenderPass::RenderPass(const std::vector<Toki::Attachment>& attachments) {
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    std::vector<VkAttachmentReference> attachmentReferences;
    std::vector<VkSubpassDependency> subpassDependencies;
    std::vector<VkSubpassDescription> subpassDescriptions;

    uint8_t foundPresentAttachmentCount = 0;

    std::optional<VkAttachmentReference> depthStenctilAttachmentReference;

    for (uint32_t i = 0; i < attachments.size(); ++i) {
        TK_ASSERT(foundPresentAttachmentCount <= 1, "More than one present attachment provided, extra presentable attachment index: {}", i);

        const Attachment& attachment = attachments[i];

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format =
            attachment.presentable ? context.physicalDeviceData.presentableSurfaceFormat.format : mapFormat(attachment.colorFormat);
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.initialLayout = /* attachment.presentable ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : */ VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescription.finalLayout = attachment.presentable ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachmentDescription.loadOp = attachmentDescription.stencilLoadOp = mapLoadOp(attachment.loadOp);
        attachmentDescription.storeOp = attachmentDescription.stencilStoreOp = mapStoreOp(attachment.storeOp);

        VkSubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;

        switch (attachment.colorFormat) {
            case ColorFormat::R8:
            case ColorFormat::RG8:
            case ColorFormat::RGBA8: {
                VkAttachmentReference attachmentReference{};
                attachmentReference.attachment = i;

                attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                attachmentDescriptions.emplace_back(attachmentDescription);
                attachmentReferences.emplace_back(attachmentReference);

                subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                subpassDependency.srcAccessMask = VK_ACCESS_NONE;
                subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            }

            case ColorFormat::Depth:
                attachmentDescriptions.emplace_back(attachmentDescription);
                attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                goto SETUP_SUBPASS_DEPENDENCY;

            case ColorFormat::Stencil:
                attachmentDescriptions.emplace_back(attachmentDescription);
                attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                goto SETUP_SUBPASS_DEPENDENCY;

            case ColorFormat::DepthStencil:
                attachmentDescriptions.emplace_back(attachmentDescription);
                attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;

            SETUP_SUBPASS_DEPENDENCY:
                subpassDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                subpassDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                subpassDependency.srcAccessMask = VK_ACCESS_NONE;
                subpassDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            default:
                std::unreachable();
        }

        subpassDependencies.emplace_back(subpassDependency);

        if (attachments[i].presentable) {
            ++foundPresentAttachmentCount;
        }
    }

    VkSubpassDescription subpassDescription{};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = attachmentReferences.size();
    subpassDescription.pColorAttachments = attachmentReferences.data();

    if (depthStenctilAttachmentReference.has_value()) {
        subpassDescription.pDepthStencilAttachment = &depthStenctilAttachmentReference.value();
    }

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = attachmentDescriptions.size();
    renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = subpassDependencies.size();
    renderPassCreateInfo.pDependencies = subpassDependencies.data();

    TK_ASSERT_VK_RESULT(
        vkCreateRenderPass(context.device, &renderPassCreateInfo, context.allocationCallbacks, &m_renderPass), "Could not create render pass");
}

RenderPass::~RenderPass() {
    vkDestroyRenderPass(context.device, m_renderPass, context.allocationCallbacks);
}

}  // namespace Toki
