#pragma once

#include <vulkan/vulkan.h>

namespace toki {

#define FRAME_COUNT 3

struct RendererContext {
    VkInstance instance{};
    VkDevice device{};
    VkPhysicalDevice physicalDevice{};

    VkAllocationCallbacks* allocationCallbacks{};
};

}  // namespace toki
