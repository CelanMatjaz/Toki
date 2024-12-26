#pragma once

#include <toki/core.h>
#include <vulkan/vulkan.h>

#include <array>

namespace toki {

#ifndef TK_DIST
static std::array validation_layers = {
    "VK_LAYER_KHRONOS_validation",
};
#endif

bool check_validation_layer_support();

bool is_device_suitable(VkPhysicalDevice physical_device);

struct QueueFamilyIndices {
    i32 graphics = -1;
    i32 present = -1;
    i32 transfer = -1;
};

QueueFamilyIndices find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

}  // namespace toki
