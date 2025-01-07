#include "engine.h"

#include <memory>

namespace toki {

engine::engine(const config& config) {
    _windows.emplace_back(config.initialWindow);

    renderer::Config renderer_config{};
    renderer_config.initialWindow = config.initialWindow;
    _renderer = renderer::create(renderer_config);
}

engine::~engine() {
    for (auto it = _views.rbegin(); it != _views.rend(); it++) {
        (*it)->on_destroy(_renderer);
    }
    _views.clear();

    _renderer.reset();
    _windows.clear();
}

void engine::run() {
    m_isRunning = true;

    static auto last_frame_time = std::chrono::high_resolution_clock::now();
    float delta_time = 0;

    while (m_isRunning) {
        window::poll_events();
        for (const auto& window : _windows) {
            if (window->should_close() && _windows.size() == 1) {
                m_isRunning = false;
            }
        }

        auto frame_start_time = std::chrono::high_resolution_clock::now();
        delta_time = std::chrono::duration<float>(frame_start_time - last_frame_time).count();
        last_frame_time = frame_start_time;

        for (auto it = _views.rbegin(); it != _views.rend(); it++) {
            (*it)->on_update(delta_time);
        }

        if (_renderer->begin_frame()) {
            for (auto it = _views.rbegin(); it != _views.rend(); it++) {
                (*it)->on_render(_renderer->get_renderer_api());
            }
            _renderer->end_frame();
            _renderer->present();
        }

        // break;
    }
}

void engine::add_view(ref<view> view) {
    view->on_add(_renderer);
    _views.emplace_back(view);
}

}  // namespace toki
