#pragma once

#include "renderer/vulkan/vulkan_context.h"
#include "renderer/vulkan/vulkan_types.h"

namespace toki {

struct create_image_config {
    VkFormat format;
    VkExtent3D extent;
    VkImageUsageFlagBits usage;
    VkMemoryPropertyFlags memory_properties;
};

vulkan_image vulkan_image_create(ref<renderer_context> ctx, const create_image_config& config);
void vulkan_image_destroy(ref<renderer_context> ctx, vulkan_image& image);

void vulkan_image_resize(ref<renderer_context> ctx, VkExtent3D extent, vulkan_image& image);
void vulkan_image_transition_layout(VkCommandBuffer cmd, VkImageLayout old_layout, VkImageLayout new_layout, vulkan_image& image);

}  // namespace toki
