// Compile file on linux while using glfw
#if defined(TK_PLATFORM_WINDOWS) || defined(TK_PLATFORM_LINUX)

#include "glfw_window.h"

#include <GLFW/glfw3.h>

#include "core/assert.h"
#include "core/core.h"
#include "core/logging.h"
#include "platform.h"

namespace toki {

static u32 window_count = 0;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

glfw_window::glfw_window(const Config& config): Window(config) {
    TK_ASSERT(config.width > 0 && config.height > 0, "Invalid window dimensions");

    if (window_count == 0) {
        glfwInit();
    }
    ++window_count;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);
    TK_ASSERT(window != nullptr, "Window was not created");

    m_handle = window;
    glfwShowWindow(window);
}

glfw_window::~glfw_window() {
    --window_count;
    if (window_count == 0) {
        glfwTerminate();
    }
}

bool glfw_window::should_close() const {
    return glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(m_handle)) == GLFW_TRUE;
}

Vec2 glfw_window::get_dimensions() const {
    int width, height;
    glfwGetFramebufferSize(m_handle, &width, &height);
    return Vec2{ width, height };
}

void* glfw_window::get_handle() const {
    return m_handle;
}

void Window::poll_events() {
    glfwPollEvents();
}

}  // namespace toki

#endif
