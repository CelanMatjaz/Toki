#include "window.h"

#include "GLFW/glfw3.h"

namespace Toki {

Window engine_create_window(const WindowConfig& config) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, config.flags & TK_WINDOW_FLAGS_RESIZABLE);
    glfwWindowHint(GLFW_VISIBLE, config.flags & TK_WINDOW_FLAGS_SHOW_ON_CREATE);
    glfwWindowHint(GLFW_FOCUSED, config.flags & TK_WINDOW_FLAGS_FOCUSED);
    glfwWindowHint(GLFW_FLOATING, config.flags & TK_WINDOW_FLAGS_FLOATING);
    glfwWindowHint(GLFW_MAXIMIZED, config.flags & TK_WINDOW_FLAGS_MAXIMIZED);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, config.flags & TK_WINDOW_FLAGS_FOCUS_ON_SHOW);

    Window window;

    window.flags = config.flags;
    window.glfw_window = glfwCreateWindow(400, 300, "Window", nullptr, nullptr);

    return window;
}

void engine_destroy_window(Window& window) {
    glfwDestroyWindow(window.glfw_window);
    window.glfw_window = nullptr;
}

}  // namespace Toki
