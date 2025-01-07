#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/error.h"
#include "renderer/vulkan/vulkan_context.h"

namespace toki {

VkSurfaceKHR create_surface(ref<renderer_context> ctx, GLFWwindow* window);

}
