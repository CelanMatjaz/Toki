#include "vulkan_image.h"

#include "core/assert.h"
#include "vulkan/vulkan_core.h"

namespace Toki {

VulkanImage::VulkanImage(const VulkanContext* context) {}

VulkanImage::~VulkanImage() {}

VkImageView VulkanImage::createImageView(const VulkanContext* context, const ImageViewConfig& c) {
    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = c.image;
    imageViewCreateInfo.viewType = c.viewType;
    imageViewCreateInfo.format = c.format;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    TK_ASSERT_VK_RESULT(
        vkCreateImageView(context->device, &imageViewCreateInfo, context->allocationCallbacks, &imageView), "Could not create image view"
    );
    return imageView;
}

}  // namespace Toki
