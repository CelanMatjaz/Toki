#pragma once

#include "renderer/vulkan/vulkan_context.h"
#include "renderer/vulkan/vulkan_types.h"

namespace toki {

struct create_buffer_config {
    u32 size;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags memory_properties;
};

vulkan_buffer vulkan_buffer_create(ref<renderer_context> ctx, const create_buffer_config& config);
void vulkan_buffer_destroy(ref<renderer_context> ctx, vulkan_buffer& buffer);

}  // namespace toki
