#include "application.h"

#include <chrono>
#include <print>

#include "assert.h"
#include "renderer/vulkan_renderer.h"
#include "toki/renderer/renderer_api/renderer_2d.h"
#include "toki/systems/font_system.h"
#include "toki/systems/ui_system.h"

namespace Toki {

Application::Application(const ApplicationConfig& config) {
    std::println("Initializing app");

    Window::initWindowSystem(this);

    m_mainWindow = Window::create(config.windowConfig);
    m_renderer = Renderer::create();
    m_renderer->init();

    Layer::s_renderer = m_renderer;
    Renderer2D::s_renderer = m_renderer;

    Renderer2D::init(m_mainWindow);

    m_renderer->createSwapchain(m_mainWindow);

    FontSystem::init(this);
    UISystem::init(this);
}

Application::~Application() {
    std::println("Deinitializing app");

    UISystem::shutdown();
    FontSystem::shutdown();

    uint32_t layerCount = m_layerStack.size();
    for (uint32_t i = 0; i < layerCount; ++i) {
        popLayer();
    }

    Renderer2D::shutdown();

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

        m_renderer->beginFrame();
        for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it) {
            (*it)->onRender();
        }
        m_renderer->endFrame();
    }

    m_renderer->waitForDevice();
}

void Application::stop() {
    m_running = true;
}

void Application::pushLayer(Ref<Layer> layer) {
    layer->m_window = m_mainWindow;
    m_layerStack.emplace_back(layer);
    m_layerStack.back()->onAttach();
}

void Application::popLayer() {
    TK_ASSERT(m_layerStack.size() > 0, "Cannot pop stack with 0 layers");
    m_layerStack.back()->onDetach();
    m_layerStack.erase(m_layerStack.end() - 1);
}

void Application::handleEvent(Event& e) {
    for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it) {
        (*it)->onEvent(e);
        if (e.isHandled()) break;
    }
}

}  // namespace Toki
