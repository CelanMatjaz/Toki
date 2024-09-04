#include "image.h"

#include <utility>

namespace Toki {

static VkImageAspectFlags get_image_aspect_flags(VkFormat format);
extern uint32_t find_memory_type(
    const VkPhysicalDeviceMemoryProperties& physical_device_memory_properties, uint32_t typeBits, VkMemoryPropertyFlags properties);

TkError create_image(VulkanState* state, const ImageConfig* image_config, RendererImage* renderer_image_out) {
    renderer_image_out->memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VkImageCreateInfo image_create_info{};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = renderer_image_out->format = image_config->format;
    image_create_info.extent = renderer_image_out->extent = image_config->extent;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = image_config->usage | VK_IMAGE_USAGE_SAMPLED_BIT;

    VkResult result = vkCreateImage(state->device, &image_create_info, state->allocation_callbacks, &renderer_image_out->image);
    ASSERT_VK_RESULT(result, Error::RENDERER_CREATE_RESOURCE_ERROR);

    ImageViewConfig image_view_config{};
    ASSERT_ERROR(create_image_view(state, &image_view_config, &renderer_image_out->image_view), "Could not create image view");

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(state->device, renderer_image_out->image, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex =
        find_memory_type(state->physical_device_data.memory_properties, memory_requirements.memoryTypeBits, renderer_image_out->memory_properties);

    result = vkAllocateMemory(state->device, &memory_allocate_info, state->allocation_callbacks, &renderer_image_out->memory);
    ASSERT_VK_RESULT(result, Error::RENDERER_CREATE_RESOURCE_ERROR);

    result = vkBindImageMemory(state->device, renderer_image_out->image, renderer_image_out->memory, 0);
    ASSERT_VK_RESULT(result, Error::RENDERER_CREATE_RESOURCE_ERROR);

    return TkError{};
}

void destroy_image(VulkanState* state, RendererImage* renderer_image) {
    vkFreeMemory(state->device, renderer_image->memory, state->allocation_callbacks);
    destroy_image_view(state, renderer_image->image_view);
    vkDestroyImage(state->device, renderer_image->image, state->allocation_callbacks);
    renderer_image->image = VK_NULL_HANDLE;
    renderer_image->image_view = VK_NULL_HANDLE;
}

TkError create_image_view(VulkanState* state, const ImageViewConfig* image_view_config, VkImageView* image_view_out) {
    VkImageViewCreateInfo image_view_create_info{};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.subresourceRange = {};
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.image = image_view_config->image;
    image_view_create_info.format = image_view_config->format;
    image_view_create_info.subresourceRange.aspectMask = get_image_aspect_flags(image_view_config->format);

    VkResult result = vkCreateImageView(state->device, &image_view_create_info, state->allocation_callbacks, &*image_view_out);
    ASSERT_VK_RESULT(result, Error::RENDERER_CREATE_RESOURCE_ERROR);

    return TkError{};
}

void destroy_image_view(VulkanState* state, VkImageView image_view) {
    vkDestroyImageView(state->device, image_view, state->allocation_callbacks);
}

static VkImageAspectFlags get_image_aspect_flags(VkFormat format) {
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

uint32_t find_memory_type(
    const VkPhysicalDeviceMemoryProperties& physical_device_memory_properties, uint32_t typeBits, VkMemoryPropertyFlags properties) {
    for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++) {
        if ((typeBits & (1 << i)) && (physical_device_memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    std::unreachable();
}

}  // namespace Toki
