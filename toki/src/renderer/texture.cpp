#include "texture.h"

#include <stb_image.h>

#include "renderer/buffer.h"
#include "renderer/command_pool.h"
#include "renderer/mapping_functions.h"
#include "renderer/vulkan_types.h"
#include "toki/core/assert.h"
#include "toki/renderer/renderer_types.h"

namespace Toki {

Texture::Texture(ColorFormat format, std::filesystem::path path) :
    m_format(mapFormat(format)),
    m_usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT),
    m_memoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
    int width, height, channels;
    int forcedChannels = STBI_grey_alpha;

    switch (format) {
        case ColorFormat::R8:
            forcedChannels = STBI_grey;
            break;

        case ColorFormat::RG8:
            forcedChannels = STBI_grey_alpha;
            break;

        case ColorFormat::RGBA8:
            forcedChannels = STBI_grey_alpha;
            break;

        case ColorFormat::Depth:
        case ColorFormat::Stencil:
        case ColorFormat::DepthStencil:
            TK_ASSERT(false, "Provided an unsupported color format for texture");
            break;

        default:
            std::unreachable();
    }

    uint32_t* pixels = (uint32_t*) stbi_load(std::filesystem::absolute(path).string().c_str(), &width, &height, &channels, forcedChannels);
    uint64_t imageSize = width * height * 4;

    TK_ASSERT(pixels != nullptr, "Error loading image");
    TK_ASSERT(imageSize > 0, "Image size is 0");

    switch (channels) {
        case 3:
        case 4:
            m_format = VK_FORMAT_R8G8B8A8_SRGB;
            break;

        default:
            std::unreachable();
    }

    createImage();
    createImageView();

    setData(imageSize, pixels);

    stbi_image_free(pixels);
}

Texture::Texture(ColorFormat format, uint32_t width, uint32_t height, uint32_t layers) :
    m_format(mapFormat(format)),
    m_extent(width, height, layers),
    m_usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT),
    m_memoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
    createImage();
    createImageView();
}

Texture::~Texture() {
    destroy();
}

void Texture::setData(uint32_t size, void* data) {
    setData(size, Region3D{ { 0.0f, 0.0f, 0.0f }, { m_extent.width, m_extent.height, m_extent.depth } }, data);
}

void Texture::setData(uint32_t size, const Region3D& region, void* data) {
    Buffer stagingBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.setData(size, 0, data);

    CommandPool::submitSingleUseCommands(context.physicalDeviceData.transferQueue, [this, b = stagingBuffer, r = region](VkCommandBuffer cmd) {
        this->transitionLayout(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { static_cast<int32_t>(r.position.x), static_cast<int32_t>(r.position.y), static_cast<int32_t>(r.position.z) };
        region.imageExtent = { static_cast<uint32_t>(r.extent.x), static_cast<uint32_t>(r.extent.y), static_cast<uint32_t>(r.extent.z) };

        vkCmdCopyBufferToImage(cmd, b.m_buffer, this->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        this->transitionLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });
}

void Texture::resize(const Extent3D& extent) {
    m_extent = { extent.x, extent.y, extent.z };

    destroy();
    createImage();
    createImageView();
}

void Texture::createImage() {
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = m_format;
    imageCreateInfo.extent = m_extent;
    imageCreateInfo.mipLevels = 0;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = m_usage | VK_IMAGE_USAGE_SAMPLED_BIT;

    vkCreateImage(context.device, &imageCreateInfo, nullptr, &m_image);

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(context.device, m_image, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, m_memoryProperties);

    TK_ASSERT_VK_RESULT(
        vkAllocateMemory(context.device, &memoryAllocateInfo, context.allocationCallbacks, &m_memory), "Could not allocate buffer memory");
    TK_ASSERT_VK_RESULT(vkBindImageMemory(context.device, m_image, m_memory, 0), "Could not bind image memory");
}

static VkImageAspectFlags getImageAspectFlags(VkFormat format) {
    switch (format) {
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_R8G8B8A8_SRGB:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        case VK_FORMAT_D32_SFLOAT:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            std::unreachable();
    }
}

void Texture::createImageView() {
    VkImageAspectFlags aspectMask = getImageAspectFlags(m_format);

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
    imageViewCreateInfo.image = m_image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    vkCreateImageView(context.device, &imageViewCreateInfo, context.allocationCallbacks, &m_imageView);
}

void Texture::destroy() {
    vkFreeMemory(context.device, m_memory, context.allocationCallbacks);
    vkDestroyImageView(context.device, m_imageView, context.allocationCallbacks);
    vkDestroyImage(context.device, m_image, context.allocationCallbacks);
}

void Texture::transitionLayout(VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkPipelineStageFlags sourceStage{};
    VkPipelineStageFlags destinationStage{};

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

WrappedImage ::WrappedImage(VkImage image, VkFormat format) {
    VkImageAspectFlags aspectMask = getImageAspectFlags(format);

    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.subresourceRange = {};
    imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    vkCreateImageView(context.device, &imageViewCreateInfo, context.allocationCallbacks, &m_imageView);
}

WrappedImage::~WrappedImage() {
    vkDestroyImageView(context.device, m_imageView, context.allocationCallbacks);
}

}  // namespace Toki
