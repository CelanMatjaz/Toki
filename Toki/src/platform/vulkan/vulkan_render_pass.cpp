#include "tkpch.h"
#include "core/assert.h"
#include "vulkan_render_pass.h"
#include "platform/vulkan/backend/vulkan_utils.h"
#include "platform/vulkan/vulkan_renderer.h"

namespace Toki {

    VulkanRenderPass::VulkanRenderPass(const RenderPassConfig& config) : RenderPass(config) {
        std::vector<VkSubpassDescription> subpasses(config.nSubpasses);
        colorAttachmentCounts.resize(config.nSubpasses, 0);

        std::vector<VkAttachmentDescription> attachments(config.attachments.size());
        bool isSwapchainTargetSet = false;
        for (uint32_t i = 0; i < config.attachments.size(); ++i) {
            // TK_ASSERT(!isSwapchainTargetSet, std::format("Only 1 RenderTarget::Swapchain attachment can exist on a framebuffer, problem with attachment with index {}", i));
            isSwapchainTargetSet = config.attachments[i].target == RenderTarget::Swapchain;

            attachments[i] = config.attachments[i].isDepthAttachment
                ? createDepthAttachmentDescription(config.attachments[i])
                : createColorAttachmentDescription(config.attachments[i], isSwapchainTargetSet);

            if (config.attachments[i].isDepthAttachment)
                hasDepthAttachment_ = true;
        }

        std::vector<std::vector<VkAttachmentReference>> colorAttachmentRefs(config.nSubpasses);
        std::vector<std::vector<VkAttachmentReference>> inputAttachmentRefs(config.nSubpasses);
        std::vector<std::vector<VkAttachmentReference>> resolveAttachmentRefs(config.nSubpasses);
        std::vector<std::vector<VkAttachmentReference>> depthReferences(config.nSubpasses);
        std::vector<std::vector<uint32_t>> preserveAttachmentRefs(config.nSubpasses);

        std::vector<VkSubpassDependency> subpassDependencies;

        auto findPrevSubpassIndex = [](SubpassBits inBits, uint8_t currentBitIndex) {
            SubpassBits bits;
            do {
                bits = inBits >> --currentBitIndex;
                if (bits & 1) return (uint32_t) currentBitIndex;
            } while (bits);

            return VK_SUBPASS_EXTERNAL;
        };

        auto getImageLayout = [](bool isDepth = false) {
            if (isDepth) return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        };

        for (uint8_t subpassIndex = 0; subpassIndex < config.nSubpasses; ++subpassIndex) {
            bool isSwapchainTargetSet = false;

            subpasses[subpassIndex] = {};
            VkSubpassDescription& subpass = subpasses[subpassIndex];
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            std::vector<VkAttachmentReference>* subpassColorAttachments = &colorAttachmentRefs[subpassIndex];
            std::vector<VkAttachmentReference>* subpassInputAttachments = &inputAttachmentRefs[subpassIndex];
            std::vector<VkAttachmentReference>* subpassResolveAttachments = &resolveAttachmentRefs[subpassIndex];
            std::vector<uint32_t>* subpassPreserveAttachments = &preserveAttachmentRefs[subpassIndex];

            for (uint32_t attachmentIndex = 0; attachmentIndex < config.attachments.size(); ++attachmentIndex) {
                const Attachment* attachment = &config.attachments[attachmentIndex];

                bool isIncluded = attachment->includeSubpassBits >> subpassIndex & 1;
                if (!isIncluded) continue;

                bool isColor = !attachment->isDepthAttachment;
                bool isInput = attachment->inputSubpassBits >> subpassIndex & 1;
                bool isResolve = attachment->resolveSubpassBits >> subpassIndex & 1;

                VkAttachmentReference ref = { attachmentIndex, getImageLayout(!isColor) };

                uint32_t srcSubpassIndex = findPrevSubpassIndex(attachment->dependantSubpassBits, subpassIndex);

                if (isColor) {
                    subpassColorAttachments->push_back(ref);
                    if (isResolve)
                        subpassResolveAttachments->push_back(ref);
                    subpassDependencies.push_back(createColorDependency(srcSubpassIndex, subpassIndex));
                    ++colorAttachmentCounts[subpassIndex];
                }
                else {
                    depthReferences[subpassIndex].push_back(ref);
                    subpass.pDepthStencilAttachment = &depthReferences[subpassIndex].back();
                    subpassDependencies.push_back(createDepthDependency(srcSubpassIndex, subpassIndex));
                }

                if (isInput)
                    subpassInputAttachments->push_back(ref);

                if (!attachment->includeSubpassBits >> (subpassIndex + 1))
                    subpassPreserveAttachments->emplace_back(attachmentIndex);
            }

            subpass.colorAttachmentCount = subpassColorAttachments->size();
            subpass.pColorAttachments = subpassColorAttachments->data();
            subpass.inputAttachmentCount = subpassInputAttachments->size();
            subpass.pInputAttachments = subpassInputAttachments->data();
            subpass.pResolveAttachments = subpassResolveAttachments->data();
            subpass.preserveAttachmentCount = subpassPreserveAttachments->size();
            subpass.pPreserveAttachments = subpassPreserveAttachments->data();
        }

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = subpasses.size();
        renderPassCreateInfo.pSubpasses = subpasses.data();
        renderPassCreateInfo.dependencyCount = subpassDependencies.size();
        renderPassCreateInfo.pDependencies = subpassDependencies.data();

        TK_ASSERT_VK_RESULT(vkCreateRenderPass(VulkanRenderer::device(), &renderPassCreateInfo, nullptr, &renderPassHandle), "Could not create render pass");
    }

    VulkanRenderPass::~VulkanRenderPass() {
        vkDeviceWaitIdle(VulkanRenderer::device());
        vkDestroyRenderPass(VulkanRenderer::device(), renderPassHandle, nullptr);
    }

    VkAttachmentDescription VulkanRenderPass::createColorAttachmentDescription(const Attachment& attachment, bool isSwapchainAttachment) {
        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = isSwapchainAttachment ? VulkanRenderer::swapchain()->getFormat() : VulkanUtils::mapFormat(attachment.format);
        attachmentDescription.samples = VulkanUtils::mapSamples(attachment.samples);
        attachmentDescription.loadOp = VulkanUtils::mapLoadOp(attachment.loadOp);
        attachmentDescription.storeOp = VulkanUtils::mapStoreOp(attachment.storeOp);
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = (attachment.initialLayout == InitialLayout::Undefined ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR); // TODO: create mapping function
        attachmentDescription.finalLayout = isSwapchainAttachment ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        return attachmentDescription;
    }

    VkAttachmentDescription VulkanRenderPass::createDepthAttachmentDescription(const Attachment& attachment) {
        VkAttachmentDescription attachmentDescription = createColorAttachmentDescription(attachment);
        attachmentDescription.format = VulkanUtils::mapFormat(attachment.format);
        attachmentDescription.stencilLoadOp = VulkanUtils::mapLoadOp(attachment.loadOp);
        attachmentDescription.stencilStoreOp = VulkanUtils::mapStoreOp(attachment.storeOp);
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        return attachmentDescription;
    }

    VkSubpassDependency VulkanRenderPass::createColorDependency(uint32_t srcSubpass, uint32_t dstSubpass) {
        VkSubpassDependency colorDependency{};
        colorDependency.srcSubpass = srcSubpass;
        colorDependency.dstSubpass = dstSubpass;
        colorDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        colorDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        colorDependency.srcAccessMask = VK_ACCESS_NONE;
        colorDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        colorDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        return colorDependency;
    }

    VkSubpassDependency VulkanRenderPass::createDepthDependency(uint32_t srcSubpass, uint32_t dstSubpass) {
        VkSubpassDependency depthDependency{};
        depthDependency.srcSubpass = srcSubpass;
        depthDependency.dstSubpass = dstSubpass;
        depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depthDependency.srcAccessMask = VK_ACCESS_NONE;
        depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        depthDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        return depthDependency;
    }

}