#include "engine.h"

#include <toki/core.h>

namespace toki {

Engine::Engine(const Config& config):
    m_global_allocator(config.memory_config.engine_memory_block_size),
    m_frame_allocator(m_global_allocator, config.memory_config.engine_frame_memory_block_size) {}

Engine::~Engine() {
    for (u32 i = 0; i < s_window_count; i++) {
        s_windows[i].destroy();
    }
}

void Engine::run() {
    m_is_running = true;

    static auto last_frame_time = platform::get_time_microseconds();
    [[maybe_unused]] float delta_time = 0;

    EventHandler h;

    while (m_is_running) {
        s_windows[0].handle_events(h);

        auto frame_start_time = platform::get_time_microseconds();
        delta_time = (frame_start_time - last_frame_time) / 1'000'000.0f;
        last_frame_time = frame_start_time;

        m_frame_allocator.swap();
        m_frame_allocator->clear();
    }
}

void Engine::add_window(const char* title, u32 width, u32 height) {
    TK_ASSERT(s_window_count < MAX_ENGINE_WINDOW_COUNT, "Adding another window would go over current limit");

    Window::Config window_config{};
    window_config.title = title;
    window_config.width = width;
    window_config.height = height;

    s_windows[s_window_count++].create(window_config);
}

}  // namespace toki
