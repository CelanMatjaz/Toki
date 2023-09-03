#pragma once 

#include "vulkan/vulkan.h"
#include "renderer/framebuffer.h"

namespace Toki {

    struct VulkanRenderPassConfig {
        std::vector<Attachment> colorAttachments;
        Ref<Attachment> depthAttachment;
    };

    class VulkanRenderPass {
    public:
        VulkanRenderPass(VulkanRenderPassConfig& config);
        ~VulkanRenderPass();

        VkRenderPass getHandle() { return handle; }

    private:
        static VkAttachmentDescription createColorAttachmentDescription(const Attachment& attachment, bool isSwapchainAttachment = false);
        static VkAttachmentDescription createDepthAttachmentDescription(const Attachment& attachment);
        static VkSubpassDependency createColorDependency();
        static VkSubpassDependency createDepthDependency();

        VkRenderPass handle = VK_NULL_HANDLE;
    };

}