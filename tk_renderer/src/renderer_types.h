#pragma once

#include <vulkan/vulkan.h>

namespace Toki {

struct PhysicalDeviceData {
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;
    VkPhysicalDeviceMemoryProperties memory_properties;
};

struct VulkanState {
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    PhysicalDeviceData physical_device_data{};
    VkAllocationCallbacks* allocation_callbacks = nullptr;

    operator VkInstance() { return instance; }
    operator VkDevice() { return device; }
    operator VkPhysicalDevice() { return physical_device_data.physical_device; }
};

inline VulkanState* s_renderer_state = nullptr;

}  // namespace Toki
