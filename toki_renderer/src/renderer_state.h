#pragma once

#include <vulkan/vulkan.h>

namespace toki {

struct RendererState {
    VkInstance instance{};
    VkDevice device{};

    VkAllocationCallbacks* allocation_callbacks{};
};

}  // namespace toki
