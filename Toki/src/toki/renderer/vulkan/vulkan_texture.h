#pragma once

#include "tkpch.h"
#include "vulkan_buffer.h"

namespace Toki {

    class VulkanTexture {
    public:
        enum ImageFormat {
            SINGLE = 1,
            RGB = 3,
            RGBA = 4
        };

    public:
        VulkanTexture() = default;
        ~VulkanTexture();

        static VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& imageMemory);
        static VkImageView createImageView(const VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags);
        static void copyBufferToTexture(std::shared_ptr<VulkanBuffer>& buffer, std::shared_ptr<VulkanTexture>& texture);
        static void copyBufferToTexture(VulkanBuffer* buffer, VulkanTexture* texture);

        static VkFormat getFormatFromEnum(ImageFormat format);

        void cleanup();
        void setData(uint32_t size, void* data);
        void transitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

        VkImage getImage() { return image; }
        VkImageView getImageView() { return imageView; };
        uint32_t getWidth() { return width; }
        uint32_t getHeight() { return height; }

        static std::shared_ptr<VulkanTexture> create(const std::filesystem::path& filePath, ImageFormat format = RGBA);
        static std::shared_ptr<VulkanTexture> create(uint32_t width, uint32_t height, ImageFormat format = RGBA);
        static std::shared_ptr<VulkanTexture> create(uint32_t width, uint32_t height, void* data, ImageFormat format = RGBA);
        static VkSampler createSampler();

    private:

        VkImage image;
        VkImageView imageView;
        VkDeviceMemory memory;
        VkFormat format;
        ImageFormat formatEnum;
        uint32_t width;
        uint32_t height;
    };

}