#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "core/core.h"
#include "renderer/renderer.h"

namespace toki {

#define FRAME_COUNT 3

inline const std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

struct Frame {
    VkCommandBuffer commandBuffer{};
};

struct RendererContext {
    VkInstance instance{};
    VkDevice device{};
    VkPhysicalDevice physicalDevice{};

    VkAllocationCallbacks* allocationCallbacks{};

    u32 currentFrameIndex{};
    Frame frames[FRAME_COUNT]{};
};

}  // namespace toki
