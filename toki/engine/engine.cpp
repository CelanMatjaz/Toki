#include "engine.h"

namespace toki {

Engine::Engine(const Config& config) {}

Engine::~Engine() {
	// shutdown(mEngineAllocator);
}

void Engine::run() {
	m_is_running = true;

	Time time;
	Time last_frame_time = time;
	f32 delta_time = 0;

	while (m_is_running) {
		Window::poll_events();

		Time frame_start_time;
		delta_time = (frame_start_time - last_frame_time).as<Time::Unit::Seconds>() / 1'000;
		last_frame_time = frame_start_time;

		m_renderer.frame_begin();

		// for (u32 i = 0; i < m_layers.size(); i++) {
		// 	m_layers[i]->on_render()
		// }

		m_renderer.present();
		m_renderer.frame_end();
	}
}

void Engine::window_add(const Window::Config& config) {
	auto window = toki::window_create(config);
	m_windows[0] = window;
	m_renderer.window_add(window);

	TK_LOG_DEBUG("Added window");
}

}  // namespace toki
