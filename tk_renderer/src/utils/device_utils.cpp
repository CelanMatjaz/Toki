#include "device_utils.h"

#include <toki/core.h>

#include <cstdint>
#include <set>

#include "vulkan/vulkan_core.h"

namespace Toki {

bool renderer_utils_check_for_validation_layer_support() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

    for (const char* required_layer : validation_layers) {
        bool layer_found = false;

        for (const auto& found_layer : layers) {
            if (strcmp(required_layer, found_layer.layerName) == 0) {
                layer_found = true;
                break;
            }
        }

        if (!layer_found) return false;
    }

    return true;
}

DeviceQueues renderer_utils_get_device_queues(VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties.data());

    std::set<DeviceQueues::QueueIndex> supports_present;
    std::set<DeviceQueues::QueueIndex> supports_graphics;
    std::set<DeviceQueues::QueueIndex> supports_transfer;

    for (uint32_t i = 0; i < queue_family_count; ++i) {
        VkBool32 supports_present_queue{};
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &supports_present_queue);

        if (supports_present_queue) {
            supports_present.insert({ i, queue_family_properties[i].queueCount });
        }

        if ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            supports_graphics.insert({ i, queue_family_properties[i].queueCount });
        }

        if ((queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)) {
            supports_transfer.insert({ i, queue_family_properties[i].queueCount });
        }
    }

    TK_ASSERT(supports_present.size() > 0, "Physical device does not support present queue");
    TK_ASSERT(supports_graphics.size() > 0, "Physical device does not support graphics queue");
    TK_ASSERT(supports_transfer.size() > 0, "Physical device does not support transfer queue");

    DeviceQueues queues;

    // TODO: Write actual queue index selection
    queues.present_indices.emplace_back(supports_present.begin()->index);
    queues.graphics_indices.emplace_back(supports_graphics.begin()->index);
    queues.transfer_indices.emplace_back(supports_transfer.begin()->index);

    return queues;
}

}  // namespace Toki
