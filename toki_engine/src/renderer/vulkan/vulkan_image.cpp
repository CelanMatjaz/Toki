#include "vulkan_image.h"

#include <vulkan/vulkan.h>

#include "renderer/vulkan/vulkan_utils.h"

namespace toki {

vulkan_image vulkan_image_create(ref<renderer_context> ctx, const create_image_config& config) {
    vulkan_image image{};
    image.extent = config.extent;
    image.format = config.format;
    image.usage = config.usage;
    image.memory_properties = config.memory_properties;

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

    VK_CHECK(vkCreateImage(ctx->device, &image_create_info, nullptr, &image.handle), "Could not create image");

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(ctx->device, image.handle, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = find_memory_type(ctx->physical_device, memory_requirements.memoryTypeBits, config.memory_properties);

    VK_CHECK(vkAllocateMemory(ctx->device, &memory_allocate_info, ctx->allocation_callbacks, &image.memory), "Could not allocate image memory");
    VK_CHECK(vkBindImageMemory(ctx->device, image.handle, image.memory, 0), "Could not bind image memory");

    VkImageViewCreateInfo image_view_create_info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    image_view_create_info.image = image.handle;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = config.format;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(ctx->device, &image_view_create_info, ctx->allocation_callbacks, &image.image_view), "Could not create image view");

    return image;
}

void vulkan_image_destroy(ref<renderer_context> ctx, vulkan_image& image) {
    vkFreeMemory(ctx->device, image.memory, ctx->allocation_callbacks);
    vkDestroyImageView(ctx->device, image.image_view, ctx->allocation_callbacks);
    image.image_view = VK_NULL_HANDLE;
    vkDestroyImage(ctx->device, image.handle, ctx->allocation_callbacks);
    image.handle = VK_NULL_HANDLE;
}

void vulkan_image_resize(ref<renderer_context> ctx, VkExtent3D extent, vulkan_image& image) {
    create_image_config config{};
    config.format = image.format;
    config.extent = extent;
    config.usage = image.usage;
    config.memory_properties = image.memory_properties;
    vulkan_image_destroy(ctx, image);
    image = vulkan_image_create(ctx, config);
}

void vulkan_image_transition_layout(VkCommandBuffer cmd, VkImageLayout old_layout, VkImageLayout new_layout, vulkan_image& image) {
    VkImageMemoryBarrier image_memory_barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    image_memory_barrier.image = image.handle;
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

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        image_memory_barrier.srcAccessMask = 0;
        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    vkCmdPipelineBarrier(cmd, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
}

}  // namespace toki
