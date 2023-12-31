#include "glfw_window.h"

#ifdef WINDOW_SYSTEM_GLFW

namespace Toki {

GlfwWindow::GlfwWindow(WindowConfig w) {
    glfwWindowHint(GLFW_RESIZABLE, w.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // windowHandle = glfwCreateWindow(w.width, w.height, w.title, nullptr, nullptr);
    windowHandle = glfwCreateWindow(w.width, w.height, w.title, nullptr, nullptr);
    glfwSetWindowAttrib(windowHandle, GLFW_RESIZABLE, w.resizable);

    glfwMakeContextCurrent(windowHandle);
    glfwSetWindowUserPointer(windowHandle, this);
}

GlfwWindow::~GlfwWindow() {}

bool GlfwWindow::shouldClose() const {
    return glfwWindowShouldClose(windowHandle);
}

void GlfwWindow::pollEvents() const {
    glfwPollEvents();
}

Window::WindowDimensions GlfwWindow::getDimensions() const {
    int32_t width, height;
    glfwGetFramebufferSize(windowHandle, &width, &height);
    return { width, height };
}

}  // namespace Toki

#endif