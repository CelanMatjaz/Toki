#pragma once

#include "tkpch.h"

namespace Toki {

    class VulkanImage {
    public:
        struct VulkanImageSpec {
            VkFormat format;
            VkImageUsageFlagBits usage;
            VkExtent3D extent;
            VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        };

        void cleanup();

        static VulkanImage create(const VulkanImageSpec& vulkanImageSpec);

        static VkImage createImage(VkFormat format, VkExtent3D extent, VkImageUsageFlags usage);
        static VkImageView createImageView(VkFormat format, VkImageAspectFlags aspectMask, VkImage image);

    private:
        VulkanImage() = default;

        VkImage image;
        VkDeviceMemory memory;
        VkImageView imageView;
        VkFormat format;
        VkExtent3D extent;
    };

}