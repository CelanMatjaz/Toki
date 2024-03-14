#include "glfw_window.h"

#ifdef TK_WINDOW_SYSTEM_GLFW

#include "toki/core/application.h"
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

    glfwSetWindowSizeLimits(m_windowHandle, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwMakeContextCurrent(m_windowHandle);
    glfwSetWindowUserPointer(m_windowHandle, this);

    glfwSetFramebufferSizeCallback(m_windowHandle, windowResizedCallback);
    glfwSetKeyCallback(m_windowHandle, keyCallback);
    glfwSetCursorPosCallback(m_windowHandle, cursorPosCallback);
    glfwSetMouseButtonCallback(m_windowHandle, mouseButtonCallback);

    glfwGetFramebufferSize(m_windowHandle, &m_dimensions.width, &m_dimensions.height);
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

#define DISPATCH_EVENT(e, ptr)     \
    s_application->handleEvent(e); \
    if (!e.isHandled()) Event::dispatchEvent(e, ptr);

void GlfwWindow::windowResizedCallback(GLFWwindow* window, int width, int height) {
    Toki::GlfwWindow* win = (GlfwWindow*) (glfwGetWindowUserPointer(window));
    win->m_dimensions.width = width;
    win->m_dimensions.height = height;

    Event e = { EventType::WindowResize, EventData{ .i32 = { width, height } } };
    DISPATCH_EVENT(e, win);
}

void GlfwWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {}

void GlfwWindow::cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
    Toki::GlfwWindow* win = (GlfwWindow*) (glfwGetWindowUserPointer(window));

    static double prevX = 0, prevY = 0;

    Event e = { EventType::MouseMove, EventData{ .i32 = { (int32_t) (xPos - prevX), (int32_t) (yPos - prevY) } } };
    DISPATCH_EVENT(e, win);

    prevX = xPos;
    prevY = yPos;
}

void GlfwWindow::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Toki::GlfwWindow* win = (GlfwWindow*) (glfwGetWindowUserPointer(window));

    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);

    switch (action) {
        case GLFW_PRESS: {
            Event e = { EventType::MousePress, EventData{ .i16 = { (int16_t) xPos, (int16_t) yPos, (int16_t) button, (int16_t) mods } } };
            DISPATCH_EVENT(e, win);
            break;
        }
        case GLFW_RELEASE: {
            Event e = { EventType::MouseRelease, EventData{ .i16 = { (int16_t) xPos, (int16_t) yPos, (int16_t) button, (int16_t) mods } } };
            DISPATCH_EVENT(e, win);
            break;
        }
    }
}

}  // namespace Toki

#endif