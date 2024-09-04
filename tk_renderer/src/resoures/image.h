#pragma once

#include <toki/core.h>

#include "renderer_types.h"
#include "vulkan/vulkan.h"

namespace Toki {

struct ImageConfig {
    VkFormat format;
    VkExtent3D extent;
    VkImageUsageFlags usage;
};

struct ImageViewConfig {
    VkImage image;
    VkFormat format;
};

[[nodiscard]] TkError create_image(VulkanState* state, const ImageConfig* image_config, RendererImage* renderer_image_out);
void destroy_image(VulkanState* state, RendererImage* renderer_image);

[[nodiscard]] TkError create_image_view(VulkanState* state, const ImageViewConfig* image_view_config, VkImageView* image_view_out);
void destroy_image_view(VulkanState* state, VkImageView image_view);

}  // namespace Toki
