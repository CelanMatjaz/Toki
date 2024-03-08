#pragma once

#include <vulkan/vulkan.h>

#include "renderer/vulkan_types.h"
#include "toki/core/core.h"

namespace Toki {

class VulkanRenderer;

struct VulkanImageConfig {
    VkFormat format;
    VkExtent3D extent;
    VkImageUsageFlagBits usage;
    VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    uint32_t mips = 1;
};

class VulkanImage {
    friend VulkanRenderer;

public:
    VulkanImage() = delete;
    // Wrap swapchain image
    VulkanImage(VkImage image, VkFormat format);
    VulkanImage(const VulkanImageConfig& config);

    ~VulkanImage();

    VkImageView getImageView();

    void setData(uint32_t size, void* data);
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
