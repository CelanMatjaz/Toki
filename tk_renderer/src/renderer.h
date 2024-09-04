#pragma once

#include <GLFW/glfw3.h>
#include <toki/core.h>
#include <vulkan/vulkan.h>

namespace Toki {

struct RendererInitConfig {
    GLFWwindow* initial_window;
};

struct FrameData {
    double delta_time = 0;
    double total_time = 0;
    uint64_t frame_number = 0;
};

[[nodiscard]] TkError renderer_initialize(const RendererInitConfig& config);
[[nodiscard]] TkError renderer_shutdown();

[[nodiscard]] TkError renderer_begin_frame(FrameData& frame_data);
[[nodiscard]] TkError renderer_end_frame(FrameData& frame_data);
[[nodiscard]] TkError renderer_present(FrameData& frame_data);

}  // namespace Toki
