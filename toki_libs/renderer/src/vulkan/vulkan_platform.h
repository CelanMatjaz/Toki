#pragma once

#include <toki/core.h>
#include <vulkan/vulkan.h>

namespace toki {

namespace renderer {

VkSurfaceKHR create_surface(VkInstance instance, VkAllocationCallbacks* allocation_callbacks, platform::NativeWindowHandle handle);

}

}  // namespace toki
