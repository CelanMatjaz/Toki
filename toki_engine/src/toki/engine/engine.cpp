#include "engine.h"

#include <memory>

namespace toki {

Engine::Engine(const Config& config) {
    m_windows.emplace_back(config.initialWindow);

    Renderer::Config renderer_config{};
    renderer_config.initialWindow = config.initialWindow;
    m_renderer = std::make_shared<Renderer>(renderer_config);
}

Engine::~Engine() {
    m_renderer = nullptr;
    m_windows.clear();
}

void Engine::run() {
    m_isRunning = true;

    while (m_isRunning) {
        Window::poll_events();
        for (const auto& window : m_windows) {
            if (window->should_close() && m_windows.size() == 1) {
                m_isRunning = false;
            }
        }
    }
}

}  // namespace toki
