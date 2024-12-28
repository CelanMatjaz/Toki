#include "window.h"

#include <GLFW/glfw3.h>

#include <memory>
#include <print>
#include <utility>

#include "core/core.h"

namespace toki {

static u32 window_count = 0;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

std::shared_ptr<Window> Window::create(const Config& config) {
    return std::make_shared<Window>(config);
}

Window::Window(const Config& config) {
    TK_ASSERT(config.width > 0 && config.height > 0, "Invalid window dimensions");

    if (window_count == 0) {
        glfwInit();
    }
    ++window_count;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
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

const void* Window::get_handle() const {
    return m_handle;
}

bool Window::should_close() const {
    return glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(m_handle)) == GLFW_TRUE;
}

void Window::poll_events() {
    glfwPollEvents();
}

}  // namespace toki
