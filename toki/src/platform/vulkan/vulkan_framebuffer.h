#pragma once

#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_types.h"

namespace Toki {

class VulkanFramebuffer {
public:
    static Ref<VulkanFramebuffer> create(const VulkanContext* context, const FramebufferConfig& framebufferConfig);

    VulkanFramebuffer() = delete;
    VulkanFramebuffer(const VulkanContext* context, const FramebufferConfig& framebufferConfig);
    ~VulkanFramebuffer();

    void recreate(uint32_t width, uint32_t height, uint32_t layers);
    void recreate(uint32_t width, uint32_t height, uint32_t layers, VkImageView swapchainExtension);

    FramebufferConfig getConfig() const { return config; }

    const VkFramebuffer getHandle() const;
    const VkExtent2D getExtent() const;

private:
    void create();
    void destroy();

    const VulkanContext* context;
    FramebufferConfig config;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkExtent2D extent{};
};

}  // namespace Toki
