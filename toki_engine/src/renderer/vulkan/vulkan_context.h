#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "containers/array_map.h"
#include "renderer/vulkan/state/vulkan_state.h"

namespace toki {

struct RendererContext {
    VkInstance instance{};
    VkDevice device{};
    VkPhysicalDevice physical_device{};
    queue_family_indices queue_family_indices{};
    queues queues{};

    VkAllocationCallbacks* allocation_callbacks{};

    VulkanSwapchain swapchain;

    std::vector<VkCommandPool> command_pools;
    std::vector<VkCommandPool> extra_command_pools;

    array_map<Handle, VulkanGraphicsPipeline> shaders;
    array_map<Handle, VulkanBuffer> buffers;
    array_map<Handle, VulkanFramebuffer> framebuffers;
};

}  // namespace toki
