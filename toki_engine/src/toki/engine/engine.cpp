#include "engine.h"

#include <memory>

#include "core/assert.h"
#include "core/base.h"
#include "core/logging.h"

namespace toki {

Engine::Engine(const Config& config) {
    Window::InternalConfig window_config;
    window_config.width = config.window_config.width;
    window_config.height = config.window_config.height;
    window_config.title = config.window_config.title;
    window_config.engine_ptr = this;
    window_config.event_dispatch_fn = Engine::handle_event;
    Ref<Window> initial_window = Window::create(window_config);
    m_windows.emplace_back(initial_window);

    Renderer::Config renderer_config{};
    renderer_config.initialWindow = initial_window;
    m_renderer = Renderer::create(renderer_config);

    m_eventHandler = create_scope<EventHandler>();
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
            (*it)->on_update(m_renderer, delta_time);
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

void Engine::handle_event(Engine* engine, Event event) {
    TK_ASSERT(!event.is_handled(), "An already handled event cannot be handled by engine");

    for (auto it = engine->m_views.rbegin(); it != engine->m_views.rend(); ++it) {
        (*it)->on_event(event);
        if (event.is_handled()) break;
    }
}

}  // namespace toki
