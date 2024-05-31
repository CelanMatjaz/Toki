#pragma once

#include <vulkan/vulkan.h>

#include <filesystem>

#include "toki/core/containers.h"
#include "toki/renderer/renderer_types.h"

namespace Toki {

struct Texture {
    Texture() = delete;
    Texture(ColorFormat format, std::filesystem::path path);
    Texture(ColorFormat format, uint32_t width, uint32_t height, uint32_t layers = 1);
    ~Texture();

    void setData(uint32_t size, void* data);
    void setData(uint32_t size, const Region3D& region, void* data);

    void resize(const Extent3D& extent);

    void createImage();
    void createImageView();
    void destroy();

    void transitionLayout(VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout);

    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkFormat m_format;
    VkExtent3D m_extent{};
    VkImageUsageFlags m_usage;
    VkMemoryPropertyFlags m_memoryProperties;
};

struct WrappedImage {
    WrappedImage() = delete;
    WrappedImage(VkImage image, VkFormat format);
    ~WrappedImage();

    VkImageView m_imageView = VK_NULL_HANDLE;
};

}  // namespace Toki
