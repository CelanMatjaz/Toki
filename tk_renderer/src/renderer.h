#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <vector>

namespace Toki {

void renderer_initialize_state();
void renderer_destroy_state();

struct RendererInitConfig {
    GLFWwindow* initial_window;
};

void renderer_initialize(const RendererInitConfig& config);
void renderer_shutdown();

}  // namespace Toki
