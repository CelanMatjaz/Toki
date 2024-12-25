#include "engine.h"

#include <memory>

namespace toki {

Engine::Engine(const Config& config) {
    Window::Config window_config{};
    window_config.width = 800;
    window_config.height = 600;
    window_config.title = "Window";
    m_window = std::make_unique<Window>(window_config);

    Renderer::Config renderer_config{};
    m_renderer = std::make_unique<Renderer>(renderer_config);
}

Engine::~Engine() {
    m_renderer = nullptr;
    m_window = nullptr;
}

void Engine::run() {
    m_isRunning = true;

    while (m_isRunning && !m_window->should_close()) {
        Window::poll_events();
    }
}

}  // namespace toki
