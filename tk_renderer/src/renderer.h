#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <toki/core.h>

namespace Toki {

struct RendererInitConfig {
    GLFWwindow* initial_window;
};

TkError renderer_initialize(const RendererInitConfig& config);
TkError renderer_shutdown();

}  // namespace Toki
