#include "vulkan_render_pass.h"

#include "toki/core/toki.h"
#include "infos.h"

namespace Toki {

    void VulkanRenderPass::cleanup() {
        VkDevice device = Application::getVulkanRenderer()->getDevice();
        vkDestroyRenderPass(device, renderPass, nullptr);
    }

    VulkanRenderPass VulkanRenderPass::create(const RenderPassSpec& spec) {
        VulkanRenderPass pass;

        VkRenderPassCreateInfo renderPassCreateInfo = Infos::RenderPass::renderPassCreateInfo(spec.attachments, spec.dependencies, spec.subpasses);
        TK_ASSERT_VK_RESULT(vkCreateRenderPass(Application::getVulkanRenderer()->getDevice(), &renderPassCreateInfo, nullptr, &pass.renderPass));

        return pass;
    }

    VkAttachmentDescription VulkanRenderPass::createColorAttachmentDescription(VkFormat format) {
        return Infos::RenderPass::colorAttachmentDescription(format);
    }

    VkAttachmentDescription VulkanRenderPass::createDepthAttachmentDescription(VkFormat format) {
        return Infos::RenderPass::depthAttachmentDescription(format);
    }

    VkSubpassDescription VulkanRenderPass::createSubpassDescription(const AttachmentReferences& attachmentReferences) {
        return Infos::RenderPass::subpassDescription(attachmentReferences.colorAttachmentReferences, attachmentReferences.depthAttachmentReference);
    }

    void VulkanRenderPass::beginRenderPass(VkCommandBuffer cmd, VkRenderPassBeginInfo* renderPassBeginInfo) {
        renderPassBeginInfo->renderPass = renderPass;
        vkCmdBeginRenderPass(cmd, renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanRenderPass::endRenderPass(VkCommandBuffer cmd) {
        vkCmdEndRenderPass(cmd);
    }

}