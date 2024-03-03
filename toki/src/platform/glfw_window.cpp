#include "glfw_window.h"

#ifdef TK_WINDOW_SYSTEM_GLFW

#include "toki/events/event.h"

namespace Toki {

GlfwWindow::GlfwWindow(const WindowConfig& windowConfig) {
    glfwWindowHint(GLFW_RESIZABLE, windowConfig.isResizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_VISIBLE, windowConfig.showOnCreate ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, windowConfig.focusOnShow ? GLFW_TRUE : GLFW_FALSE);
    if (windowConfig.showOnCreate) {
        glfwWindowHint(GLFW_FOCUSED, windowConfig.focusOnCreate ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_FLOATING, windowConfig.floatingOnCreate ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_MAXIMIZED, windowConfig.maximizedOnCreate ? GLFW_TRUE : GLFW_FALSE);
    }

    m_windowHandle = glfwCreateWindow(windowConfig.width, windowConfig.height, windowConfig.title.c_str(), nullptr, nullptr);

    glfwMakeContextCurrent(m_windowHandle);
    glfwSetWindowUserPointer(m_windowHandle, this);

    glfwSetFramebufferSizeCallback(m_windowHandle, windowResizedCallback);
    glfwSetKeyCallback(m_windowHandle, keyCallback);
}

GlfwWindow::~GlfwWindow() {}

void GlfwWindow::pollEvents() {
    glfwPollEvents();
}

bool GlfwWindow::shouldClose() {
    return glfwWindowShouldClose(m_windowHandle);
}

void GlfwWindow::show() {
    glfwShowWindow(m_windowHandle);
}

void GlfwWindow::hide() {
    glfwHideWindow(m_windowHandle);
}

void GlfwWindow::close() {
    glfwSetWindowShouldClose(m_windowHandle, GLFW_TRUE);
}

void GlfwWindow::resize(uint16_t width, uint16_t height) {
    glfwSetWindowSize(m_windowHandle, width, height);
}

void GlfwWindow::setTitle(std::string_view title) {
    glfwSetWindowTitle(m_windowHandle, title.data());
}

void GlfwWindow::setFloating(bool floating) {
    glfwSetWindowAttrib(m_windowHandle, GLFW_FLOATING, floating ? GLFW_TRUE : GLFW_FALSE);
}

void* GlfwWindow::getHandle() {
    return m_windowHandle;
}

void GlfwWindow::windowResizedCallback(GLFWwindow* window, int width, int height) {
    Toki::GlfwWindow* win = (GlfwWindow*) (glfwGetWindowUserPointer(window));
    win->m_dimensions.width = width;
    win->m_dimensions.height = height;

    Event::dispatchEvent(Event{ EventType::WindowResize, EventData{ .i32 = { width, height } } }, win);
}

void GlfwWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {}

}  // namespace Toki

#endif