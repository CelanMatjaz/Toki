#include "vulkan_image.h"

#include <utility>

#include "renderer/vulkan/vulkan_utils.h"

namespace toki {

void VulkanImage::create(Ref<RendererContext> ctx, const Config& config) {
    m_extent = config.extent;
    m_format = config.format;
    m_usage = config.usage;
    m_memoryProperties = config.memory_properties;

    VkImageCreateInfo image_create_info{};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = config.format;
    image_create_info.extent = config.extent;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = config.usage | VK_IMAGE_USAGE_SAMPLED_BIT;

    VK_CHECK(vkCreateImage(ctx->device, &image_create_info, nullptr, &m_handle), "Could not create image");

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(ctx->device, m_handle, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = find_memory_type(ctx->physical_device, memory_requirements.memoryTypeBits, config.memory_properties);

    VK_CHECK(vkAllocateMemory(ctx->device, &memory_allocate_info, ctx->allocation_callbacks, &m_memory), "Could not allocate image memory");
    VK_CHECK(vkBindImageMemory(ctx->device, m_handle, m_memory, 0), "Could not bind image memory");

    VkImageViewCreateInfo image_view_create_info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    image_view_create_info.image = m_handle;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = config.format;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(ctx->device, &image_view_create_info, ctx->allocation_callbacks, &m_imageView), "Could not create image view");
}

void VulkanImage::destroy(Ref<RendererContext> ctx) {
    vkFreeMemory(ctx->device, m_memory, ctx->allocation_callbacks);
    vkDestroyImageView(ctx->device, m_imageView, ctx->allocation_callbacks);
    m_imageView = VK_NULL_HANDLE;
    vkDestroyImage(ctx->device, m_handle, ctx->allocation_callbacks);
    m_handle = VK_NULL_HANDLE;
}

void VulkanImage::resize(Ref<RendererContext> ctx, VkExtent3D extent) {
    destroy(ctx);
    Config config{};
    config.format = m_format;
    config.extent = extent;
    config.usage = m_usage;
    config.memory_properties = m_memoryProperties;
    create(ctx, config);
}

void VulkanImage::transition_layout(VkCommandBuffer cmd, VkImageLayout old_layout, VkImageLayout new_layout) {
    transition_layout(cmd, old_layout, new_layout, m_handle);
}

void VulkanImage::transition_layout(VkCommandBuffer cmd, VkImageLayout old_layout, VkImageLayout new_layout, VkImage image) {
    VkImageMemoryBarrier image_memory_barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    image_memory_barrier.image = image;
    image_memory_barrier.oldLayout = old_layout;
    image_memory_barrier.newLayout = new_layout;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_barrier.subresourceRange.baseMipLevel = 0;
    image_memory_barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0;
    image_memory_barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        image_memory_barrier.srcAccessMask = 0;
        image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destination_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        image_memory_barrier.srcAccessMask = 0;
        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else {
        std::unreachable();
    }

    vkCmdPipelineBarrier(cmd, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
}

VkImage VulkanImage::get_handle() const {
    return m_handle;
}

VkImageView VulkanImage::get_image_view() const {
    return m_imageView;
}

}  // namespace toki
