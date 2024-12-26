#include "window.h"

#include <GLFW/glfw3.h>
#include <toki/core.h>

#include <print>

namespace toki {

static u32 window_count = 0;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

Window::Window(const Config& config) {
    TK_ASSERT(config.width > 0 && config.height > 0, "Invalid window dimensions");

    if (window_count == 0) {
        glfwInit();
    }
    ++window_count;

    GLFWwindow* window =
        glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);

    m_handle = window;
    glfwShowWindow(window);
}

Window::~Window() {
    --window_count;
    if (window_count == 0) {
        glfwTerminate();
    }
}

bool Window::should_close() const {
    return glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(m_handle)) == GLFW_TRUE;
}

void Window::poll_events() {
    glfwPollEvents();
}

}  // namespace toki
