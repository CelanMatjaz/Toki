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

    Layer::s_renderer = m_renderer;

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

    while (m_running) {
        m_mainWindow->pollEvents();

        if (m_mainWindow->shouldClose()) break;

        auto frameStartTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(frameStartTime - lastFrameTime).count();
        lastFrameTime = frameStartTime;

        if (m_renderer->beginFrame()) {
            for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it) {
                (*it)->onRender();
            }

            m_renderer->endFrame();
        }
    }
}

void Application::stop() {
    m_running = true;
}

void Application::pushLayer(Ref<Layer> layer) {
    m_layerStack.emplace_back(layer);
    m_layerStack.back()->onAttach();
}

void Application::popLayer() {
    m_layerStack.back()->onDetach();
    m_layerStack.erase(m_layerStack.end() - 1);
}

}  // namespace Toki
