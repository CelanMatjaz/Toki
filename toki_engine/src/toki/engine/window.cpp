#include "window.h"

#include "core/logging.h"

#ifdef TK_WINDOW_SYSTEM_GLFW
#include "platform/glfw_window.h"
#endif

namespace toki {

Window::Window(const InternalConfig& config): m_enginePtr(config.engine_ptr), m_eventDispatchFn(config.event_dispatch_fn) {}

Ref<Window> Window::create(const InternalConfig& config) {
    TK_LOG_INFO("Creating new window");
#ifdef TK_WINDOW_SYSTEM_GLFW
    return create_ref<GlfwWindow>(config);
#endif
}

const Ref<Input> Window::get_input() const {
    return m_input;
}

}  // namespace toki
