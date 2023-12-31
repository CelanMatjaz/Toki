#pragma once

#include "vulkan/vulkan_types.h"

namespace Toki {

class VulkanRenderPass {
public:
    VulkanRenderPass(VulkanContext* context, const RenderPassConfig& renderPassConfig);
    ~VulkanRenderPass();

    VkRenderPass getHandle() { return renderPass; }

    void createFramebuffer();
    void createPipeline();

private:
    void create();
    void destroy();

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VulkanContext* context = nullptr;
    RenderPassConfig config{};

    VkAttachmentDescription createAttachmentDescription();
};

}  // namespace Toki
