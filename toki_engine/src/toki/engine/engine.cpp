#include "engine.h"

#include <memory>

namespace toki {

Engine::Engine(const Config& config) {
    m_windows.emplace_back(config.initialWindow);

    Renderer::Config renderer_config{};
    renderer_config.initialWindow = config.initialWindow;
    m_renderer = Renderer::create(renderer_config);
}

Engine::~Engine() {
    for (auto it = m_views.rbegin(); it != m_views.rend(); it++) {
        (*it)->on_destroy(m_renderer);
    }
    m_views.clear();

    m_renderer.reset();
    m_windows.clear();
}

void Engine::run() {
    m_isRunning = true;

    static auto last_frame_time = std::chrono::high_resolution_clock::now();
    float delta_time = 0;

    while (m_isRunning) {
        Window::poll_events();
        for (const auto& window : m_windows) {
            if (window->should_close() && m_windows.size() == 1) {
                m_isRunning = false;
            }
        }

        auto frame_start_time = std::chrono::high_resolution_clock::now();
        delta_time = std::chrono::duration<float>(frame_start_time - last_frame_time).count();
        last_frame_time = frame_start_time;

        for (auto it = m_views.rbegin(); it != m_views.rend(); it++) {
            (*it)->on_update(delta_time);
        }

        if (m_renderer->begin_frame()) {
            for (auto it = m_views.rbegin(); it != m_views.rend(); it++) {
                (*it)->on_render(m_renderer->get_renderer_api());
            }
            m_renderer->end_frame();
            m_renderer->present();
        }
    }

    m_renderer->wait_for_resources();
}

void Engine::add_view(Ref<View> view) {
    view->on_add(m_renderer);
    m_views.emplace_back(view);
}

}  // namespace toki
