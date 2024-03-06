#include "vulkan_image.h"

#include <toki/core/assert.h>

#include "renderer/vulkan_utils.h"
#include "vulkan/vulkan_core.h"

namespace Toki {

VulkanImage::VulkanImage(VkImage image, VkFormat format) : m_imageHandle(image), m_format(format), m_isWrapped(true) {
    createImageView();
}

VulkanImage::~VulkanImage() {
    vkDestroyImageView(s_context->device, m_imageViewHandle, s_context->allocationCallbacks);

    if (m_isWrapped) {
        return;
    }

    vkFreeMemory(s_context->device, m_memoryHandle, s_context->allocationCallbacks);
    vkDestroyImage(s_context->device, m_imageHandle, s_context->allocationCallbacks);
}

VkImageView VulkanImage::getImageView() {
    return m_imageViewHandle;
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

    // VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    // VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    // VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL -> VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        TK_ASSERT(false, "Old and new layout combination is not supported");
    }

    vkCmdPipelineBarrier(cmd, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
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

}  // namespace Toki
