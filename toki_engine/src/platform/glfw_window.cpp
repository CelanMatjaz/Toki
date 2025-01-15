#if defined(TK_WINDOW_SYSTEM_GLFW)

#include "glfw_window.h"

#include <GLFW/glfw3.h>

#include <utility>

#include "core/assert.h"
#include "core/math_types.h"
#include "events/event.h"

namespace toki {

static u32 window_count = 0;

void Window::poll_events() {
    glfwPollEvents();
}

GlfwWindow::GlfwWindow(const InternalConfig& config): Window(config) {
    TK_ASSERT(config.width > 0 && config.height > 0, "Invalid window dimensions");

    if (window_count == 0) {
        glfwInit();
    }
    ++window_count;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, config.flags.resizable ? GLFW_TRUE : GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);
    TK_ASSERT(window != nullptr, "Window was not created");

    m_handle = window;
    m_input = create_ref<Input>(m_handle);

    glfwShowWindow(m_handle);

    glfwSetWindowUserPointer(m_handle, this);

    // Window callbacks
    glfwSetKeyCallback(m_handle, key_callback);

    glfwSetMouseButtonCallback(m_handle, mouse_button_callback);
    glfwSetCursorPosCallback(m_handle, mouse_move_callback);
    glfwSetCursorEnterCallback(m_handle, mouse_enter_callback);
    glfwSetScrollCallback(m_handle, scroll_callback);

    glfwSetWindowCloseCallback(m_handle, window_close_callback);
    glfwSetWindowPosCallback(m_handle, window_move_callback);
    glfwSetWindowSizeCallback(m_handle, window_resize_callback);
    glfwSetFramebufferSizeCallback(m_handle, window_resize_callback);
    glfwSetWindowMaximizeCallback(m_handle, window_maximize_callback);
    glfwSetWindowFocusCallback(m_handle, window_focus_callback);
}

GlfwWindow::~GlfwWindow() {
    --window_count;
    if (window_count == 0) {
        glfwTerminate();
    }
}

b8 GlfwWindow::should_close() const {
    return glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(m_handle)) == GLFW_TRUE;
}

Vec2 GlfwWindow::get_dimensions() const {
    int width, height;
    glfwGetFramebufferSize(m_handle, &width, &height);
    return Vec2{ width, height };
}

void* GlfwWindow::get_handle() const {
    return m_handle;
}

#define DISPATCH_WINDOW_EVENT(event)                                      \
    {                                                                     \
        GlfwWindow* win = (GlfwWindow*) glfwGetWindowUserPointer(window); \
        win->m_eventDispatchFn(win->m_enginePtr, event);                  \
    }

void GlfwWindow::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    EventType type;
    switch (action) {
        case GLFW_PRESS:
            type = EventType::KeyPress;
            break;
        case GLFW_RELEASE:
            type = EventType::KeyRelease;
            break;
        case GLFW_REPEAT:
            type = EventType::KeyRepeat;
            break;
        default:
            std::unreachable();
    }

    DISPATCH_WINDOW_EVENT(create_key_event(type, key, scancode, action, mods));
}

void GlfwWindow::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    EventType type;
    switch (action) {
        case GLFW_PRESS:
            type = EventType::MouseClick;
            break;
        case GLFW_RELEASE:
            type = EventType::MouseRelease;
            break;
        case GLFW_REPEAT:
            type = EventType::MouseHold;
            break;
        default:
            std::unreachable();
    }

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    DISPATCH_WINDOW_EVENT(create_mouse_button_event(type, button, action, mods, xpos, ypos));
}

void GlfwWindow::mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
    DISPATCH_WINDOW_EVENT(create_mouse_move_event(xpos, ypos));
}

void GlfwWindow::mouse_enter_callback(GLFWwindow* window, int entered) {
    if (entered) {
        DISPATCH_WINDOW_EVENT(Event(EventType::MouseEnter));
    } else {
        DISPATCH_WINDOW_EVENT(Event(EventType::MouseLeave));
    }
}

void GlfwWindow::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    DISPATCH_WINDOW_EVENT(create_mouse_scroll_event(xoffset, yoffset));
}

void GlfwWindow::window_close_callback(GLFWwindow* window) {
    DISPATCH_WINDOW_EVENT(Event(EventType::WindowClose));
}

void GlfwWindow::window_move_callback(GLFWwindow* window, int xpos, int ypos) {
    DISPATCH_WINDOW_EVENT(create_window_move_event(xpos, ypos));
}

void GlfwWindow::window_resize_callback(GLFWwindow* window, int width, int height) {
    DISPATCH_WINDOW_EVENT(create_window_resize_event(width, height));
}

void GlfwWindow::window_maximize_callback(GLFWwindow* window, int maximized) {
    if (maximized) {
        DISPATCH_WINDOW_EVENT(Event(EventType::WindowMaximize));
    } else {
        DISPATCH_WINDOW_EVENT(Event(EventType::WindowRestore));
    }
}

void GlfwWindow::window_iconify_callback(GLFWwindow* window, int iconified) {
    if (iconified) {
        DISPATCH_WINDOW_EVENT(Event(EventType::WindowMaximize));
    } else {
        DISPATCH_WINDOW_EVENT(Event(EventType::WindowRestore));
    }
}

void GlfwWindow::window_focus_callback(GLFWwindow* window, int focused) {
    if (focused) {
        DISPATCH_WINDOW_EVENT(Event(EventType::WindowFocus));
    } else {
        DISPATCH_WINDOW_EVENT(Event(EventType::WindowBlur));
    }
}

}  // namespace toki

#endif
