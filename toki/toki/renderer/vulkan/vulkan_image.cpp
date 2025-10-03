#include <vulkan/vulkan_core.h>

#include "toki/core/common/assert.h"
#include "toki/renderer/private/vulkan//vulkan_backend.h"

namespace toki::renderer {

VkImageView VulkanBackend::create_image_view(const ImageViewConfig& config) {
	VkImageViewCreateInfo image_view_create_info{};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.image = config.image;
	image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_view_create_info.subresourceRange.baseMipLevel = 0;
	image_view_create_info.subresourceRange.levelCount = 1;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.layerCount = 1;

	VkImageView image_view;
	VkResult result =
		vkCreateImageView(m_state.device, &image_view_create_info, m_state.allocation_callbacks, &image_view);
	TK_ASSERT(result == VK_SUCCESS);

	return image_view;
}

}  // namespace toki::renderer
