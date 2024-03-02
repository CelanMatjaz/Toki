#include "application.h"

#include <print>

#include "platform/glfw_window.h"

namespace Toki {

Application::Application(const ApplicationConfig& config) {
    std::println("Initializing app");

    m_mainWindow = createScope<GlfwWindow>(config.windowConfig);
}

Application::~Application() {
    std::println("Deinitializing app");
}

void Application::start() {
    m_mainWindow->show();
    std::println("Starting app");

    while (!m_mainWindow->shouldClose()) {
        m_mainWindow->pollEvents();
    }
}

void Application::stop() {}

}  // namespace Toki
