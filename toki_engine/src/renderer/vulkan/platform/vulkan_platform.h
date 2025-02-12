#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "engine/window.h"

namespace toki {

VkSurfaceKHR create_surface(VkInstance instance, VkAllocationCallbacks* allocation_callbacks, Window* window);

}
