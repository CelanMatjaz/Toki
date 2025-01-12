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
    QueueFamilyIndices queue_family_indices{};
    Queues queues{};

    VkAllocationCallbacks* allocation_callbacks{};

    VulkanSwapchain swapchain;

    std::vector<VkCommandPool> command_pools;
    std::vector<VkCommandPool> extra_command_pools;

    std::unordered_map<Handle, VulkanGraphicsPipeline> shaders;
    std::unordered_map<Handle, VulkanBuffer> buffers;
    std::unordered_map<Handle, VulkanFramebuffer> framebuffers;
};

}  // namespace toki
