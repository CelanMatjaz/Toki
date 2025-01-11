#if defined(TK_WINDOW_SYSTEM_GLFW)

#pragma once

#include <GLFW/glfw3.h>

#include "engine/window.h"

namespace toki {

// This class name will be the reason for so many bugs...
class GlfwWindow : public Window {
public:
    GlfwWindow() = delete;
    GlfwWindow(const InternalConfig& config);
    virtual ~GlfwWindow();

    virtual Vec2 get_dimensions() const override;

    virtual b8 should_close() const override;
    virtual void* get_handle() const override;

private:
    GLFWwindow* m_handle;

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
    static void mouse_enter_callback(GLFWwindow* window, int entered);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    static void window_close_callback(GLFWwindow* window);
    static void window_move_callback(GLFWwindow* window, int xpos, int ypos);
    static void window_resize_callback(GLFWwindow* window, int width, int height);
    static void window_maximize_callback(GLFWwindow* window, int maximized);
    static void window_iconify_callback(GLFWwindow* window, int iconified);
    static void window_focus_callback(GLFWwindow* window, int focused);
};

}  // namespace toki

#endif
