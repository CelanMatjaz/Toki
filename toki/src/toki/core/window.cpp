#include "window.h"

#include <utility>

#include "platform.h"

#ifdef TK_WINDOW_SYSTEM_GLFW
#include "platform/glfw_window.h"
#endif

namespace Toki {

Ref<Window> Window::create(const WindowConfig& config) {
#ifdef TK_WINDOW_SYSTEM_GLFW
    return createRef<GlfwWindow>(config);
#else
    std::unreachable();
    return nullptr;
#endif
}

const WindowDimensions& Window::getDimensions() {
    return m_dimensions;
}

void Window::initWindowSystem(Application* application) {
#ifdef TK_WINDOW_SYSTEM_GLFW
    glfwInit();
#endif

    s_application = application;
}

void Window::shutdownWindowSystem() {
#ifdef TK_WINDOW_SYSTEM_GLFW
    glfwTerminate();
#endif
}
}  // namespace Toki