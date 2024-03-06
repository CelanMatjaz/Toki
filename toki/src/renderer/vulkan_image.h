#pragma once

#include <vulkan/vulkan.h>

#include "renderer/vulkan_types.h"
#include "toki/core/core.h"

namespace Toki {

class VulkanRenderer;

class VulkanImage {
    friend VulkanRenderer;

public:
    VulkanImage() = delete;
    // Wrap swapchain image
    VulkanImage(VkImage image, VkFormat format);
    ~VulkanImage();

    VkImageView getImageView();

    void transitionLayout(VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout);

private:
    void createImageView();

    inline static Ref<VulkanContext> s_context;

    VkImage m_imageHandle = VK_NULL_HANDLE;
    VkImageView m_imageViewHandle = VK_NULL_HANDLE;
    VkDeviceMemory m_memoryHandle = VK_NULL_HANDLE;
    VkFormat m_format;
    bool m_isWrapped : 1 = false;
};

}  // namespace Toki
