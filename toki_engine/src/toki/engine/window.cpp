#include "window.h"

#include "core/logging.h"

#ifdef TK_WINDOW_SYSTEM_GLFW
#include "platform/glfw_window.h"
#endif

namespace toki {

Window::Window(const InternalConfig& config) {};

Window* Window::create(const InternalConfig& config) {
    TK_LOG_INFO("Creating new window");
#ifdef TK_WINDOW_SYSTEM_GLFW
    return new GlfwWindow(config);
#endif
}

const Input& Window::get_input() const {
    return *m_input;
}

EventHandler& Window::get_event_handler() {
    return m_eventHandler;
}

}  // namespace toki
