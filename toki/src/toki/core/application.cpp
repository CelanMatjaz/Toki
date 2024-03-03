#include "application.h"

#include <chrono>
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

    static auto lastFrameTime = std::chrono::high_resolution_clock::now();
    float deltaTime = 0;

    while (!m_mainWindow->shouldClose()) {
        m_mainWindow->pollEvents();

        auto frameStartTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(frameStartTime - lastFrameTime).count();
        lastFrameTime = frameStartTime;

        m_renderer->beginFrame();

        m_renderer->endFrame();
    }
}

void Application::stop() {}

}  // namespace Toki
