#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "vulkan/vulkan_core.h"

namespace toki {

#define FRAME_COUNT 3

inline const std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

struct RendererContext {
    VkInstance instance{};
    VkDevice device{};
    VkPhysicalDevice physicalDevice{};

    VkAllocationCallbacks* allocationCallbacks{};
};

}  // namespace toki
