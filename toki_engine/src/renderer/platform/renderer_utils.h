#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/error.h"
#include "renderer/renderer.h"

namespace toki {

TkError create_surface(RendererContext* ctx, GLFWwindow* window, VkSurfaceKHR* surface_out);

}
