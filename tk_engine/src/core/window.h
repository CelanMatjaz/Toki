#pragma once

#include <GLFW/glfw3.h>

#include <string>

namespace Toki {

enum WindowFlags {
    // TK_WINDOW_FLAGS_VSYNC_ENABLE = 1 << 0,
    TK_WINDOW_FLAGS_RESIZABLE = 1 << 1,
    TK_WINDOW_FLAGS_SHOW_ON_CREATE = 1 << 2,
    TK_WINDOW_FLAGS_FOCUSED = 1 << 3,
    TK_WINDOW_FLAGS_FLOATING = 1 << 4,
    TK_WINDOW_FLAGS_MAXIMIZED = 1 << 5,
    TK_WINDOW_FLAGS_FOCUS_ON_SHOW = 1 << 6,
    TK_WINDOW_FLAGS_DECORATED = 1 << 7,
};

struct WindowConfig {
    std::string title;
    uint32_t width = 0, height = 0;
    uint64_t flags = TK_WINDOW_FLAGS_DECORATED | TK_WINDOW_FLAGS_SHOW_ON_CREATE;
};

struct Window {
    GLFWwindow* glfw_window;
    uint64_t flags;
};

Window engine_create_window(const WindowConfig& config);
void engine_destroy_window(Window& window);

}  // namespace Toki
