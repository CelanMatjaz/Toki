#pragma once

#include <toki/core.h>

#include "GLFW/glfw3.h"
#include "renderer_types.h"

namespace Toki {

[[nodiscard]] TkError create_instance(VulkanState* state);
void destroy_instance(VulkanState* state);

[[nodiscard]] TkError create_device(VulkanState* state, GLFWwindow* window);
void destroy_device(VulkanState* state);

[[nodiscard]] TkError create_frames(VulkanState* state, RendererFrame* frame_data);
void destroy_frames(VulkanState* state, RendererFrame* frame_data);

}  // namespace Toki
