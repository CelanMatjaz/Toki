#if defined(TK_WINDOW_SYSTEM_GLFW)

#include <GLFW/glfw3.h>

#include "input/input.h"

namespace toki {

b8 Input::is_key_down(KeyCode key_code) {
    return glfwGetKey(((GLFWwindow*) m_data), std::to_underlying(key_code)) == GLFW_PRESS;
}

b8 Input::is_mouse_button_pressed(MouseButton mouse_button) {
    return glfwGetMouseButton((GLFWwindow*) m_data, std::to_underlying(mouse_button)) == GLFW_PRESS;
}

}  // namespace toki

#endif
