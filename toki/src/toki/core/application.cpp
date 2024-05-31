#include "application.h"

#include <chrono>
#include <print>

#include "renderer/vulkan_renderer.h"
#include "toki/core/assert.h"
#include "toki/core/core.h"
#include "toki/core/frame_data.h"
#include "toki/core/logging.h"

namespace Toki {

Application::Application(const ApplicationConfig& config) {
    LOG_INFO("Initializing app");

    Window::initWindowSystem(this);

    m_mainWindow = Window::create(config.windowConfig);
    s_renderer = createRef<VulkanRenderer>();
    s_renderer->init();
    s_renderer->bindWindow(m_mainWindow);
}

Application::~Application() {
    LOG_INFO("Deinitializing app");

    uint32_t layerCount = m_layerStack.size();
    for (uint32_t i = 0; i < layerCount; ++i) {
        popLayer();
    }

    s_renderer->shutdown();
    Window::shutdownWindowSystem();
}

void Application::start() {
    m_mainWindow->show();

    LOG_INFO("Starting app");

    static auto lastFrameTime = std::chrono::high_resolution_clock::now();
    float deltaTime = 0;

    while (m_running) {
        m_mainWindow->pollEvents();

        if (m_mainWindow->shouldClose()) break;

        auto frameStartTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(frameStartTime - lastFrameTime).count();
        lastFrameTime = frameStartTime;

        static FrameData frameData{};
        frameData.deltaTime = deltaTime;
        frameData.totalTime += deltaTime;
        frameData.frameNumber++;

        s_renderer->beginFrame(frameData);

        for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it) {
            (*it)->onRender();
        }

        s_renderer->endFrame(frameData);

        s_renderer->present(frameData);

        // return;
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
    TK_ASSERT(m_layerStack.size() > 0, "Cannot pop stack with 0 layers");
    m_layerStack.back()->onDetach();
    m_layerStack.erase(m_layerStack.end() - 1);
}

void Application::handleEvent(Event& e) {
    if (e.isHandled()) return;

    for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it) {
        (*it)->onEvent(e);
        if (e.isHandled()) break;
    }
}

Renderer& Application::getRenderer() {
    return *s_renderer;
}

}  // namespace Toki
