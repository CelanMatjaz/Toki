#pragma once

#include <toki/engine.h>
#include <vulkan/vulkan.h>

namespace toki {

struct Renderer::RendererState {
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    VkAllocationCallbacks* allocation_callbacks = nullptr;

    operator VkInstance() {
        return instance;
    }

    operator VkDevice() {
        return device;
    }
};

}  // namespace toki
