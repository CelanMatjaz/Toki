#include "image_view.h"

#include <vulkan/vulkan.h>

#include "renderer/macros.h"

namespace toki {

VkImageView create_image_view(const RendererContext* state, const ImageViewConfig& config) {
    VkImageViewCreateInfo image_view_create_info{};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = config.image;
    image_view_create_info.viewType = config.viewType;

    image_view_create_info.format = config.format;
    image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    VkImageView image_view{};

    TK_ASSERT_VK_RESULT(
        vkCreateImageView(
            state->device, &image_view_create_info, state->allocationCallbacks, &image_view),
        "Could not create image view");

    return image_view;
}

}  // namespace toki
