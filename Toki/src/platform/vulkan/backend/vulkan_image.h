#pragma once

#include "vulkan/vulkan.h"
#include "platform/vulkan/vulkan_buffer.h"
#include "platform/vulkan/backend/vulkan_image.h"

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

        uint32_t getWidth() { return extent.width; };
        uint32_t getHeight() { return extent.height; };

        void setData(uint32_t size, void* data);

        void transitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToTexture(VulkanBuffer* buffer, VulkanImage* texture);

        static VkSampler createSampler();

    private:
        VkImage image = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkFormat format;
        VkExtent3D extent;

        bool isWrapped = false;
    };

}