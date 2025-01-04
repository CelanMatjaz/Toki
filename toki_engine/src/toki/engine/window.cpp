#include "window.h"

#include "platform/windows/glfw_window.h"

namespace toki {

Window::Window(const Config& config) {}

Window::~Window() {}

Ref<Window> Window::create(const Config& config) {
    return std::make_shared<GlfwWindow>(config);
}

}  // namespace toki
