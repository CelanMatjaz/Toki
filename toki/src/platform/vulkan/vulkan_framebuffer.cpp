#include "vulkan_framebuffer.h"

#include "core/assert.h"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_types.h"

namespace Toki {

Ref<VulkanFramebuffer> VulkanFramebuffer::create(const VulkanContext* context, const FramebufferConfig& framebufferConfig) {
    return createRef<VulkanFramebuffer>(context, framebufferConfig);
}

VulkanFramebuffer::VulkanFramebuffer(const VulkanContext* context, const FramebufferConfig& config)
    : context(context),
      config(config),
      extent(config.width, config.height) {
    create();
}

VulkanFramebuffer::~VulkanFramebuffer() {
    destroy();
}

void VulkanFramebuffer::recreate(uint32_t width, uint32_t height, uint32_t layers) {
    destroy();
    config.width = width;
    config.height = height;
    config.layers = layers;
    create();
}

void VulkanFramebuffer::recreate(uint32_t width, uint32_t height, uint32_t layers, VkImageView swapchainExtension) {
    if (swapchainExtension != VK_NULL_HANDLE) config.attachments[0] = swapchainExtension;
    recreate(width, height, layers);
}

const VkFramebuffer VulkanFramebuffer::getHandle() const {
    return framebuffer;
}

const VkExtent2D VulkanFramebuffer::getExtent() const {
    return extent;
}

void VulkanFramebuffer::create() {
    VkFramebufferCreateInfo framebufferCreateInfo{};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = config.renderPass;
    framebufferCreateInfo.width = config.width;
    framebufferCreateInfo.height = config.height;
    framebufferCreateInfo.layers = config.layers;
    framebufferCreateInfo.attachmentCount = config.attachments.size();
    framebufferCreateInfo.pAttachments = config.attachments.data();

    extent = { config.width, config.height };

    TK_ASSERT_VK_RESULT(
        vkCreateFramebuffer(context->device, &framebufferCreateInfo, context->allocationCallbacks, &framebuffer), "Could not create framebuffer"
    );
}

void VulkanFramebuffer::destroy() {
    vkDestroyFramebuffer(context->device, framebuffer, context->allocationCallbacks);
}

}  // namespace Toki
