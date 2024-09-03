#pragma once

#include <toki/core.h>
#include "GLFW/glfw3.h"
#include "renderer_types.h"

namespace Toki {

TkError create_instance(VulkanState* state);
TkError destroy_instance(VulkanState* state);

TkError create_device(VulkanState* state, GLFWwindow* window);
TkError destroy_device(VulkanState* state);

}  // namespace Toki
