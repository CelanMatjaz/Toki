#include "window.h"

#include "core/logging.h"
#include "platform/glfw_window.h"

namespace toki {

Window::Window(const InternalConfig& config): m_enginePtr(config.engine_ptr), m_eventDispatchFn(config.event_dispatch_fn) {}

Ref<Window> Window::create(const InternalConfig& config) {
    TK_LOG_INFO("Creating new window");
    return std::make_shared<GlfwWindow>(config);
}

}  // namespace toki
