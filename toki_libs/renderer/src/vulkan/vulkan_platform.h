#pragma once

#include <toki/core.h>
#include <vulkan/vulkan.h>

namespace toki {

VkSurfaceKHR vulkan_surface_create(
    VkInstance instance, VkAllocationCallbacks* allocation_callbacks, NativeWindowHandle handle);

}  // namespace toki
