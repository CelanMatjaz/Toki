#pragma once

#include "renderer/vulkan/vulkan_context.h"
#include "renderer/vulkan/vulkan_types.h"

namespace toki {

struct create_vulkan_swapchain_config {
    ref<window> window;
    VkCommandPool command_pool;
};

vulkan_swapchain vulkan_swapchain_create(ref<renderer_context> ctx, const create_vulkan_swapchain_config& config);
void vulkan_swapchain_destroy(ref<renderer_context> ctx, vulkan_swapchain& swapchain);
void vulkan_swapchain_recreate(ref<renderer_context> ctx, vulkan_swapchain& swapchain);

b8 vulkan_swapchain_start_recording(ref<renderer_context> ctx, vulkan_swapchain& swapchain);
void vulkan_swapchain_stop_recording(ref<renderer_context> ctx, vulkan_swapchain& swapchain);

}  // namespace toki
