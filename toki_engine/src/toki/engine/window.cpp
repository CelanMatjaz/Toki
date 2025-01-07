#include "window.h"

#include "core/logging.h"
#include "platform/glfw_window.h"

namespace toki {

Window::Window(const Config& config) {}

Window::~Window() {}

Ref<Window> Window::create(const Config& config) {
    TK_LOG_INFO("Creating new window");
    return std::make_shared<glfw_window>(config);
}

}  // namespace toki
