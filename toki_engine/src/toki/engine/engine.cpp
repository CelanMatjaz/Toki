#include "engine.h"

#include <string>

#include "core/assert.h"
#include "core/base.h"
#include "core/logging.h"
#include "memory/allocators/stack_allocator.h"

namespace toki {

Engine::Engine(const Config& config):
    m_engineAllocator(Kilobytes(16)),
    m_systemManager(nullptr),
    m_systemAllocator(Megabytes(20)) {
    Window* initial_window = Window::create(config.window_config);
    m_windows.emplace_back(initial_window);

    Renderer::Config renderer_config{};
    renderer_config.initial_window = initial_window;
    m_renderer = m_engineAllocator.emplace<Renderer>(renderer_config);

    // m_systemManager = m_systemAllocator.emplace<SystemManager>(&m_systemAllocator, m_renderer);

    initial_window->m_eventHandler.bind_all(this, [this](void*, void*, Event& event) {
        this->handle_event(event);
    });
}

Engine::~Engine() {
    for (auto it = m_views.rbegin(); it != m_views.rend(); it++) {
        (*it)->on_destroy();
        delete *it;
    }
    m_views.clear();

    m_renderer->~Renderer();

    for (auto& window : m_windows) {
        delete window;
    }
    m_windows.clear();
}

void Engine::run() {
    m_isRunning = true;

    static auto last_frame_time = std::chrono::high_resolution_clock::now();
    float delta_time = 0;

    while (m_isRunning) {
        Window::poll_events();

        auto frame_start_time = std::chrono::high_resolution_clock::now();
        delta_time = std::chrono::duration<float>(frame_start_time - last_frame_time).count();
        last_frame_time = frame_start_time;

        for (auto it = m_views.rbegin(); it != m_views.rend(); it++) {
            (*it)->on_update(delta_time);
        }

        m_renderer->begin_frame();
        for (auto it = m_views.rbegin(); it != m_views.rend(); it++) {
            (*it)->on_render();
        }
        m_renderer->present();
        m_renderer->end_frame();
    }

    m_renderer->wait_for_resources();
}

void Engine::add_view(View* view) {
    view->m_engine = this;
    view->on_add();
    m_views.emplace_back(view);
}

void Engine::handle_event(Event& event) {
    TK_ASSERT(!event.is_handled(), "An already handled event cannot be handled by engine");

    if (event == EventType::WindowClose) {
        m_isRunning = false;
        return;
    }

    for (auto it = m_views.rbegin(); it != m_views.rend(); ++it) {
        (*it)->on_event(event);
        if (event.is_handled()) {
            break;
        }
    }
}

}  // namespace toki
