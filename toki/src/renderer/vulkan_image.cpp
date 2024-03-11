#include "vulkan_image.h"

#include <stb_image.h>
#include <toki/core/assert.h>

#include <filesystem>

#include "renderer/vulkan_buffer.h"
#include "renderer/vulkan_utils.h"
#include "toki/resources/resource_utils.h"
#include "vulkan/vulkan_core.h"

namespace Toki {

VulkanImage::VulkanImage(VkImage image, VkFormat format) : m_imageHandle(image), m_format(format), m_isWrapped(true) {
    createImageView();
}

VulkanImage::VulkanImage(const VulkanImageConfig& config) : m_isWrapped(false), m_format(config.format), m_config{ config } {
    createImage();
    createImageView();
};

VulkanImage::VulkanImage(std::filesystem::path path, const TextureConfig& config) : Texture(path, config), m_isWrapped(false) {
    TK_ASSERT(ResourceUtils::fileExists(path), "File does not exist");

    int width, height, channels;
    int forcedChannels;

    // TODO: use dynamic channel count
    uint32_t* pixels = (uint32_t*) stbi_load(std::filesystem::absolute(path).string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
    VkDeviceSize imageSize = width * height * 4;

    TK_ASSERT(pixels != nullptr, "Error loading image");
    TK_ASSERT(imageSize > 0, "Image size is 0");

    m_config = {};
    m_config.extent = { (uint32_t) width, (uint32_t) height, 1 };
    m_config.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    m_config.mips = 1;
    m_config.tiling = VK_IMAGE_TILING_OPTIMAL;
    m_config.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    switch (channels) {
        case 3:
        case 4:
            m_format = VK_FORMAT_R8G8B8A8_SRGB;
            m_config.format = VK_FORMAT_R8G8B8A8_SRGB;
            break;
    }

    createImage();
    createImageView();

    setData(imageSize, pixels);

    stbi_image_free(pixels);
}

VulkanImage::~VulkanImage() {
    destroy();
}

VkImageView VulkanImage::getImageView() {
    return m_imageViewHandle;
}

void VulkanImage::resize(uint32_t width, uint32_t height, uint32_t layers) {
    m_config.extent = { width, height, layers };
    destroy();
    createImage();
    createImageView();
}

void VulkanImage::setData(uint32_t size, void* data) {
    VulkanBuffer stagingBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.setData(size, data);

    s_context->submitSingleUseCommands([this, buffer = stagingBuffer.getHandle()](VkCommandBuffer cmd) {
        this->transitionLayout(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = this->m_config.extent;

        vkCmdCopyBufferToImage(cmd, buffer, this->m_imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        this->transitionLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });
}

void VulkanImage::transitionLayout(VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_imageHandle;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        TK_ASSERT(false, "Old and new layout combination is not supported");
    }

    vkCmdPipelineBarrier(cmd, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanImage::createImage() {
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = m_config.format;
    imageCreateInfo.extent = m_config.extent;
    imageCreateInfo.mipLevels = m_config.mips;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = m_config.tiling;
    imageCreateInfo.usage = m_config.usage | VK_IMAGE_USAGE_SAMPLED_BIT;

    vkCreateImage(s_context->device, &imageCreateInfo, nullptr, &m_imageHandle);

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(s_context->device, m_imageHandle, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex =
        VulkanUtils::findMemoryType(s_context->physicalDevice, memoryRequirements.memoryTypeBits, m_config.memoryProperties);

    TK_ASSERT_VK_RESULT(vkAllocateMemory(s_context->device, &memoryAllocateInfo, nullptr, &m_memoryHandle), "Could not allocate Vulkan image memory");
    TK_ASSERT_VK_RESULT(vkBindImageMemory(s_context->device, m_imageHandle, m_memoryHandle, 0), "Could not bind Vulkan image memory");
}

void VulkanImage::createImageView() {
    VkImageAspectFlags aspectMask;
    VkImageLayout imageLayout;

    if (m_isWrapped) {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    } else {
        aspectMask = VulkanUtils::getImageAspectFlags(m_format);
    }

    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.format = m_format;
    imageViewCreateInfo.subresourceRange = {};
    imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    imageViewCreateInfo.image = m_imageHandle;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    vkCreateImageView(s_context->device, &imageViewCreateInfo, s_context->allocationCallbacks, &m_imageViewHandle);
}

void VulkanImage::destroy() {
    vkDestroyImageView(s_context->device, m_imageViewHandle, s_context->allocationCallbacks);

    if (m_isWrapped) {
        return;
    }

    vkFreeMemory(s_context->device, m_memoryHandle, s_context->allocationCallbacks);
    vkDestroyImage(s_context->device, m_imageHandle, s_context->allocationCallbacks);
}

}  // namespace Toki
