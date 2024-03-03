#include "application.h"

#include <print>

#include "renderer/vulkan_renderer.h"

namespace Toki {

Application::Application(const ApplicationConfig& config) {
    std::println("Initializing app");

    Window::initWindowSystem();

    m_mainWindow = Window::create(config.windowConfig);
    m_renderer = Renderer::create();
    m_renderer->init();

    m_renderer->createSwapchain(m_mainWindow);
}

Application::~Application() {
    std::println("Deinitializing app");

    m_renderer->shutdown();
    Window::shutdownWindowSystem();
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
