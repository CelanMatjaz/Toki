#include "render_pass.h"

#include <vulkan/vulkan.h>

#include "core/base.h"
#include "core/logging.h"
#include "renderer/vulkan/macros.h"
#include "renderer/vulkan/renderer_state.h"
#include "renderer/vulkan/utils/mapping_functions.h"

namespace toki {

RenderPass RenderPass::create(Ref<RendererContext> ctx, const Config& config) {
    std::vector<VkAttachmentDescription> attachment_descriptions;
    std::vector<VkAttachmentReference> attachment_references;
    std::vector<VkSubpassDependency> subpass_dependencies;
    std::vector<VkSubpassDescription> subpass_descriptions;

    for (u32 i = 0; i < config.attachments.size(); i++) {
        const Attachment& attachment = config.attachments[i];

        VkAttachmentDescription attachment_description{};
        attachment_description.format = attachment.presentable ? config.presentFormat : map_format(attachment.colorFormat);
        attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_description.finalLayout =
            attachment.presentable ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachment_description.loadOp = attachment_description.stencilLoadOp = map_attachment_load_op(attachment.loadOp);
        attachment_description.storeOp = attachment_description.stencilStoreOp = map_attachment_load_op(attachment.storeOp);

        VkSubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;

        switch (attachment.colorFormat) {
            case ColorFormat::R8:
            case ColorFormat::RGBA8: {
                VkAttachmentReference attachment_reference{};
                attachment_reference.attachment = i;
                attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                attachment_descriptions.emplace_back(attachment_description);
                attachment_references.emplace_back(attachment_reference);

                subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                subpassDependency.srcAccessMask = VK_ACCESS_NONE;
                subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            }

            case ColorFormat::DepthStencil:
                attachment_descriptions.emplace_back(attachment_description);
                attachment_description.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;

SETUP_SUBPASS_DEPENDENCY:
                subpassDependency.srcStageMask =
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                subpassDependency.dstStageMask =
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                subpassDependency.srcAccessMask = VK_ACCESS_NONE;
                subpassDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            default:
                std::unreachable();
        }

        subpass_dependencies.emplace_back(subpassDependency);
    }

    VkSubpassDescription subpass_description{};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = attachment_references.size();
    subpass_description.pColorAttachments = attachment_references.data();

    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = attachment_descriptions.size();
    render_pass_create_info.pAttachments = attachment_descriptions.data();
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_description;
    render_pass_create_info.dependencyCount = subpass_dependencies.size();
    render_pass_create_info.pDependencies = subpass_dependencies.data();

    RenderPass render_pass{ VK_NULL_HANDLE, { config.attachments } };
    TK_ASSERT_VK_RESULT(
        vkCreateRenderPass(ctx->device, &render_pass_create_info, ctx->allocationCallbacks, &render_pass.renderPass),
        "Could not create render pass");

    return render_pass;
}

void RenderPass::cleanup(Ref<RendererContext> ctx, RenderPass& render_pass) {
    vkDestroyRenderPass(ctx->device, render_pass.renderPass, ctx->allocationCallbacks);
    render_pass.renderPass = VK_NULL_HANDLE;
}

}  // namespace toki
