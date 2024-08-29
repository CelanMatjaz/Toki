#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace Toki {

struct DeviceQueues {
    struct QueueIndex {
        uint32_t index = 0, count = 0;
        friend bool operator<(const QueueIndex& lhs, const QueueIndex& rhs) { return lhs.index < rhs.index && lhs.count < rhs.count; }
    };

    std::vector<QueueIndex> graphics_indices;
    std::vector<QueueIndex> present_indices;
    std::vector<QueueIndex> transfer_indices;

    std::vector<VkQueue> graphics_queues;
    std::vector<VkQueue> present_queues;
    std::vector<VkQueue> transfer_queues;
};

struct PhysicalDeviceData {
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;
    VkPhysicalDeviceMemoryProperties memory_properties;
};

struct _dataa {
    std::vector<VkQueueFamilyProperties> queue_family_properties;
    std::vector<VkPresentModeKHR> present_modes;
    std::vector<VkBool32> supports_present;
    std::vector<VkSurfaceFormatKHR> surface_formats;
    VkSurfaceCapabilitiesKHR surface_capabilities;
    VkSurfaceFormatKHR presentable_surface_format;
    VkPresentModeKHR present_mode;

    DeviceQueues* device_queues;
};

struct VulkanContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    PhysicalDeviceData physical_device_data{};

    VkAllocationCallbacks* allocation_callbacks = nullptr;

    operator VkInstance() { return instance; }
    operator VkDevice() { return device; }
    operator VkPhysicalDevice() { return physical_device_data.physical_device; }
};

inline VulkanContext s_context;

}  // namespace Toki
