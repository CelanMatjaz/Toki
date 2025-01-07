#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "containers/array_map.h"
#include "renderer/vulkan/vulkan_types.h"

namespace toki {

struct renderer_context {
    VkInstance instance{};
    VkDevice device{};
    VkPhysicalDevice physical_device{};
    queue_family_indices queue_family_indices{};
    queues queues{};

    VkAllocationCallbacks* allocation_callbacks{};

    vulkan_swapchain swapchain;

    std::vector<VkCommandPool> command_pools;
    std::vector<VkCommandPool> extra_command_pools;

    array_map<handle, vulkan_graphics_pipeline> shaders;
    array_map<handle, vulkan_buffer> buffers;
    array_map<handle, vulkan_framebuffer> framebuffers;

    VkCommandBuffer get_current_command_buffer() const {
        return swapchain.frames[swapchain.current_frame].command.handle;
    }
};

}  // namespace toki
