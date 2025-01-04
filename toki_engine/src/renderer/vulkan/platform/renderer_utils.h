#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../renderer_state.h"
#include "core/error.h"

namespace toki {

VkSurfaceKHR create_surface(RendererContext* ctx, GLFWwindow* window);

}
