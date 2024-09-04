#include "device_utils.h"

#include <toki/core.h>
#include <vulkan/vulkan.h>

#include <cstring>
#include <vector>

#include "renderer_types.h"
#include "swapchain.h"
#include "vulkan/vulkan_core.h"

namespace Toki {

TkError create_surface(VulkanState* state, GLFWwindow* window, RendererWindow* renderer_window) {
    VkResult result = glfwCreateWindowSurface(state->instance, window, state->allocation_callbacks, &renderer_window->surface);
    ASSERT_VK_RESULT(result, Error::RENDERER_CREATE_SURFACE_ERROR);

    return TkError{};
}

TkError query_layer_support(const char* layers[], uint32_t layer_count, bool* supported) {
    uint32_t instance_layer_count{};
    VkResult result = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
    ASSERT_VK_RESULT(result, Error::RENDERER_INIT_ERROR);

    std::vector<VkLayerProperties> instance_layers(instance_layer_count);
    result = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data());
    ASSERT_VK_RESULT(result, Error::RENDERER_INIT_ERROR);

    for (uint32_t i = 0; i < layer_count; ++i) {
        bool layer_found = false;

        for (uint32_t layer_index = 0; layer_index < instance_layers.size(); ++layer_index) {
            if (strcmp(layers[i], instance_layers[layer_index].layerName) == 0) {
                layer_found = true;
                break;
            }
        }

        if (!layer_found) {
            *supported = false;
            return TkError{ Error::RENDERER_DEVICE_PROPERTY_ERROR };
        }
    }

    return TkError{};
}

void destroy_renderer_window(VulkanState* state, RendererWindow* window) {
    destroy_swapchain(state, window);
    if (window->surface) {
        vkDestroySurfaceKHR(state->instance, window->surface, state->allocation_callbacks);
    }
}

TkError query_physical_devices(VulkanState* state) {
    uint32_t physical_device_count{};
    VkResult result = vkEnumeratePhysicalDevices(state->instance, &physical_device_count, nullptr);
    ASSERT_VK_RESULT(result, Error::RENDERER_DEVICE_PROPERTY_ERROR);

    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    result = vkEnumeratePhysicalDevices(state->instance, &physical_device_count, physical_devices.data());
    ASSERT_VK_RESULT(result, Error::RENDERER_DEVICE_PROPERTY_ERROR);

    state->physical_device_data.physical_device = physical_devices[0];

    return TkError{};
}

TkError query_physical_device_properties(PhysicalDeviceData* physical_device_data) {
    vkGetPhysicalDeviceFeatures(physical_device_data->physical_device, &physical_device_data->device_features);
    vkGetPhysicalDeviceProperties(physical_device_data->physical_device, &physical_device_data->device_properties);
    vkGetPhysicalDeviceMemoryProperties(physical_device_data->physical_device, &physical_device_data->memory_properties);

    return TkError{};
}

TkError query_physical_device_queues(PhysicalDeviceData* physical_device_data, VkSurfaceKHR surface) {
    uint32_t queue_family_count{};
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_data->physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_data->physical_device, &queue_family_count, queue_family_properties.data());

    VkBool32 supports_present;
    VkResult result;

    for (uint32_t i = 0; i < queue_family_properties.size(); ++i) {
        result = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_data->physical_device, i, surface, &supports_present);
        ASSERT_VK_RESULT(result, Error::RENDERER_DEVICE_PROPERTY_ERROR);

        if ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && physical_device_data->graphics_queue_family_index == UINT32_MAX) {
            physical_device_data->graphics_queue_family_index = i;
            continue;
        }

        if ((queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && physical_device_data->transfer_queue_family_index == UINT32_MAX) {
            physical_device_data->transfer_queue_family_index = i;
            continue;
        }

        if (supports_present && physical_device_data->present_queue_family_index == UINT32_MAX) {
            physical_device_data->present_queue_family_index = i;
            continue;
        }
    }

    TK_ASSERT(physical_device_data->present_queue_family_index != UINT32_MAX, "Physical device does not support present queue");
    TK_ASSERT(physical_device_data->graphics_queue_family_index != UINT32_MAX, "Physical device does not support graphics queue");
    TK_ASSERT(physical_device_data->transfer_queue_family_index != UINT32_MAX, "Physical device does not support transfer queue");

    return TkError{};
}

TkError query_physical_device_present_modes(PhysicalDeviceData* physical_device_data, VkSurfaceKHR surface) {
    uint32_t present_mode_count{};
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_data->physical_device, surface, &present_mode_count, nullptr);
    ASSERT_VK_RESULT(result, Error::RENDERER_DEVICE_PROPERTY_ERROR);

    physical_device_data->present_modes.resize(present_mode_count);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device_data->physical_device, surface, &present_mode_count, physical_device_data->present_modes.data());
    ASSERT_VK_RESULT(result, Error::RENDERER_DEVICE_PROPERTY_ERROR);

    return TkError{};
}

TkError query_physical_device_surface_formats(PhysicalDeviceData* physical_device_data, VkSurfaceKHR surface) {
    uint32_t surface_format_count{};
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_data->physical_device, surface, &surface_format_count, nullptr);
    ASSERT_VK_RESULT(result, Error::RENDERER_DEVICE_PROPERTY_ERROR);

    physical_device_data->surface_formats.resize(surface_format_count);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device_data->physical_device, surface, &surface_format_count, physical_device_data->surface_formats.data());
    ASSERT_VK_RESULT(result, Error::RENDERER_DEVICE_PROPERTY_ERROR);

    return TkError{};
}

VkSurfaceFormatKHR select_surface_format(PhysicalDeviceData* physical_device_data) {
    for (const auto& format : physical_device_data->surface_formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    return physical_device_data->surface_formats.front();
}

VkPresentModeKHR select_present_mode(PhysicalDeviceData* physical_device_data) {
    for (const auto& present_mode : physical_device_data->present_modes) {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceCapabilitiesKHR query_surface_capabilities(PhysicalDeviceData* physical_device_data, VkSurfaceKHR surface) {
    VkSurfaceCapabilitiesKHR surface_capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_data->physical_device, surface, &surface_capabilities);
    return surface_capabilities;
}

}  // namespace Toki
