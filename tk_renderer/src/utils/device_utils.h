#pragma once

#include "renderer_types.h"

namespace Toki {


inline const std::vector<const char*> validation_layers = {};

bool renderer_utils_check_for_validation_layer_support();

DeviceQueues renderer_utils_get_device_queues(VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface);


}  // namespace Toki
