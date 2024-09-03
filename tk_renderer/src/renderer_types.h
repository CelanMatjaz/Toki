#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "GLFW/glfw3.h"
#include "vulkan/vulkan_core.h"

#define ASSERT_VK_RESULT(result, error)             \
    if (result != VK_SUCCESS) {                     \
        return TkError{ error, (uint64_t) result }; \
    }

namespace Toki {

inline static const uint32_t MAX_FRAMES = 3;

struct PhysicalDeviceData {
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;
    VkPhysicalDeviceMemoryProperties memory_properties;

    std::vector<VkPresentModeKHR> present_modes;
    std::vector<VkSurfaceFormatKHR> surface_formats;

    uint32_t present_queue_family_index = UINT32_MAX;
    uint32_t graphics_queue_family_index = UINT32_MAX;
    uint32_t transfer_queue_family_index = UINT32_MAX;

    VkQueue present_queue = VK_NULL_HANDLE;
    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue transfer_queue = VK_NULL_HANDLE;
};

struct RendererWindow {
    GLFWwindow* window = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkSwapchainKHR old_swapchain = VK_NULL_HANDLE;
    VkExtent2D extent{};
};

struct VulkanState {
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    PhysicalDeviceData physical_device_data{};
    VkAllocationCallbacks* allocation_callbacks = nullptr;

    operator VkInstance() { return instance; }
    operator VkDevice() { return device; }
    operator VkPhysicalDevice() { return physical_device_data.physical_device; }

    std::vector<RendererWindow> windows;
};

}  // namespace Toki
