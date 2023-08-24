#pragma once

#include "vulkan/vulkan.h"

namespace Toki {

    struct VulkanImageConfig {
        VkFormat format;
        VkExtent3D extent;
        VkImageUsageFlagBits usage;
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
        uint32_t mips = 1;
    };

    class VulkanImage {
    public:
        VulkanImage(const VulkanImageConfig& config);
        VulkanImage(VkImage image, VkImageView imageView, VkFormat format);
        ~VulkanImage();

        VkImage getImage() { return image; }
        VkImageView getView() { return view; }
        VkFormat getFormat() { return format; }

    private:
        VkImage image = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkFormat format;

        bool isWrapped = false;
    };

}