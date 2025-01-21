#include "vulkan_image.h"

#include <utility>

#include "renderer/vulkan/state/vulkan_buffer.h"
#include "renderer/vulkan/vulkan_commands.h"
#include "renderer/vulkan/vulkan_utils.h"
#include "vulkan/vulkan_core.h"

namespace toki {

void VulkanImage::create(RendererContext* ctx, const Config& config) {
    m_extent = config.extent;
    m_format = config.format;
    m_usage = config.usage;
    m_memoryProperties = config.memory_properties;

    switch (config.format) {
        case VK_FORMAT_D32_SFLOAT:
            m_aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
            break;
        case VK_FORMAT_S8_UINT:
            m_aspectFlags = VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            m_aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        default:
            m_aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    }

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
    image_view_create_info.subresourceRange.aspectMask = m_aspectFlags;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(ctx->device, &image_view_create_info, ctx->allocation_callbacks, &m_imageView), "Could not create image view");
}

void VulkanImage::destroy(RendererContext* ctx) {
    vkFreeMemory(ctx->device, m_memory, ctx->allocation_callbacks);
    vkDestroyImageView(ctx->device, m_imageView, ctx->allocation_callbacks);
    m_imageView = VK_NULL_HANDLE;
    vkDestroyImage(ctx->device, m_handle, ctx->allocation_callbacks);
    m_handle = VK_NULL_HANDLE;
}

void VulkanImage::resize(RendererContext* ctx, VkExtent3D extent) {
    destroy(ctx);
    Config config{};
    config.format = m_format;
    config.extent = extent;
    config.usage = m_usage;
    config.memory_properties = m_memoryProperties;
    create(ctx, config);
}

void VulkanImage::transition_layout(VkCommandBuffer cmd, VkImageLayout old_layout, VkImageLayout new_layout) {
    transition_layout(cmd, old_layout, new_layout, m_handle, m_aspectFlags);
}

void VulkanImage::transition_layout(VkCommandBuffer cmd, VkImageLayout old_layout, VkImageLayout new_layout, VkImage image, VkImageAspectFlags aspect_flags) {
    VkImageMemoryBarrier image_memory_barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    image_memory_barrier.image = image;
    image_memory_barrier.oldLayout = old_layout;
    image_memory_barrier.newLayout = new_layout;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.subresourceRange.aspectMask = aspect_flags;
    image_memory_barrier.subresourceRange.baseMipLevel = 0;
    image_memory_barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0;
    image_memory_barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    // TODO: rewrite
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        image_memory_barrier.srcAccessMask = VK_ACCESS_NONE;
        image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destination_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    //
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
        image_memory_barrier.srcAccessMask = VK_ACCESS_NONE;
        image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    //
    else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        image_memory_barrier.srcAccessMask = VK_ACCESS_NONE;
        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    //
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        image_memory_barrier.srcAccessMask = VK_ACCESS_NONE;
        image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    //
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        image_memory_barrier.srcAccessMask = VK_ACCESS_NONE;
        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    //
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    //
    else {
        std::unreachable();
    }

    vkCmdPipelineBarrier(cmd, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
}

void VulkanImage::transition_layout(VkCommandBuffer cmd, VkImageLayout old_layout, VkImageLayout new_layout, VulkanImage& image) {
    transition_layout(cmd, old_layout, new_layout, image.m_handle, image.m_aspectFlags);
}

void VulkanImage::set_data(RendererContext* ctx, u32 size, void* data) {
    VulkanBuffer::Config buffer_config{};
    buffer_config.size = size;
    buffer_config.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_config.memory_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VulkanBuffer staging_buffer{};
    staging_buffer.create(ctx, buffer_config);
    staging_buffer.set_data(ctx, size, data);

    vulkan_commands::submit_single_use_command_buffer(ctx, [this, &staging_buffer](VkCommandBuffer cmd) {
        this->transition_layout(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkBufferImageCopy buffer_image_copy{};
        buffer_image_copy.bufferOffset = 0;
        buffer_image_copy.bufferRowLength = 0;
        buffer_image_copy.bufferImageHeight = 0;
        buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        buffer_image_copy.imageSubresource.mipLevel = 0;
        buffer_image_copy.imageSubresource.baseArrayLayer = 0;
        buffer_image_copy.imageSubresource.layerCount = 1;
        buffer_image_copy.imageOffset = {};
        buffer_image_copy.imageExtent = this->m_extent;

        vkCmdCopyBufferToImage(cmd, staging_buffer.get_handle(), this->m_handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy);

        this->transition_layout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });

    staging_buffer.destroy(ctx);
}

VkImage VulkanImage::get_handle() const {
    return m_handle;
}

VkImageView VulkanImage::get_image_view() const {
    return m_imageView;
}

}  // namespace toki
