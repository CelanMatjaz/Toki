#pragma once

#include <toki/core.h>

#include "renderer_types.h"

namespace Toki {

TkError create_surface(VulkanState* state, GLFWwindow* window, RendererWindow* renderer_window);

TkError query_layer_support(const char* layers[], uint32_t layer_count, bool* supported);

void destroy_renderer_window(VulkanState* state, RendererWindow* window);

TkError query_physical_devices(VulkanState* state);
TkError query_physical_device_properties(PhysicalDeviceData* physical_device_data);
TkError query_physical_device_queues(PhysicalDeviceData* physical_device_data, VkSurfaceKHR surface);
TkError query_physical_device_present_modes(PhysicalDeviceData* physical_device_data, VkSurfaceKHR surface);
TkError query_physical_device_surface_formats(PhysicalDeviceData* physical_device_data, VkSurfaceKHR surface);

VkSurfaceFormatKHR select_surface_format(PhysicalDeviceData* physical_device_data);
VkPresentModeKHR select_present_mode(PhysicalDeviceData* physical_device_data);
VkSurfaceCapabilitiesKHR query_surface_capabilities(PhysicalDeviceData* physical_device_data, VkSurfaceKHR surface);

}  // namespace Toki
