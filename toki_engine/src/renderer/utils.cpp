#include "utils.h"

#include <vulkan/vulkan.h>

#include <cstring>
#include <vector>

#include "vulkan/vulkan_core.h"

namespace toki {

bool check_validation_layer_support() {
    uint32_t layer_count{};
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

bool is_device_suitable(VkPhysicalDevice physical_device) {
    return true;
}

QueueFamilyIndices find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    u32 queue_family_count{};
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device, &queue_family_count, queue_families.data());

    QueueFamilyIndices indices{};
    for (int i = 0; i < queue_families.size(); i++) {
        VkBool32 supports_present = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &supports_present);
        if (supports_present && indices.present == -1) {
            indices.present = i;
        }

        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && indices.graphics == -1) {
            indices.graphics = i;
        }

        if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT && indices.transfer == -1) {
            indices.transfer = i;
        }
    }

    TK_ASSERT(
        indices.present > -1 && indices.graphics > -1 && indices.transfer > -1,
        "GPU does not support all required queues");

    return indices;
}

}  // namespace toki
