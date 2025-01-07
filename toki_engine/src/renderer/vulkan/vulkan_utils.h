#pragma once

#include <vulkan/vulkan.h>

#include <array>

#include "renderer/renderer_types.h"
#include "renderer/vulkan/vulkan_context.h"

namespace toki {

#ifndef TK_DIST
static std::array validation_layers = {
    "VK_LAYER_KHRONOS_validation",
};
#endif

bool check_validation_layer_support();
bool is_device_suitable(VkPhysicalDevice physical_device);

queue_family_indices find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
u32 find_memory_type(VkPhysicalDevice physical_device, u32 type_filter, VkMemoryPropertyFlags properties);

// Shaders
VkShaderModule create_shader_module(ref<renderer_context> ctx, std::vector<u32>& binary);
std::vector<u32> compile_shader(shader_stage stage, std::string& source);

VkFormat map_format(color_format format);
VkAttachmentLoadOp map_attachment_load_op(render_target_load_op op);
VkAttachmentStoreOp map_attachment_store_op(render_target_store_op op);
VkBufferUsageFlags map_buffer_type(buffer_type type);
VkMemoryPropertyFlags map_buffer_memory_properties(buffer_usage usage);


}  // namespace toki
