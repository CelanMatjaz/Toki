#include "toki/runtime/engine/engine.h"

namespace toki {

Engine::Engine([[maybe_unused]] const EngineConfig& config): m_running(true) {
	WindowConfig window_config{};
	window_config.title = "Window";
	window_config.dimensions = { 800, 600 };
	window_config.min_dimensions = { 400, 300 };
	window_config.flags = WINDOW_FLAG_SHOW_ON_CREATE;
	m_window = toki::make_unique<Window>(window_config);

	RendererConfig renderer_config{};
	renderer_config.window = m_window.get();
	m_renderer = Renderer::create(renderer_config);
}

Engine::~Engine() {
	cleanup();
}

void Engine::run() {
	toki::println("Starting application");

	Time now = Time::now();
	Time previous_time = now;
	while (m_running) {
		window_system_poll_events();

		EventQueue& event_queue = m_window->get_event_queue();
		for (Event& event : event_queue) {
			if (event.type() == EventType::NONE) {
				continue;
			}
			for (i32 i = static_cast<i32>(m_layers.size() - 1); i >= 0; i--) {
				m_layers[static_cast<u32>(i)]->on_event(m_window.get(), event);
				if (event.handled()) {
					break;
				}
			}
		}
		event_queue.clear();

		now = Time::now();
		f64 delta_time = (now - previous_time).as<TimePrecision::Seconds>();
		previous_time = now;

		m_renderer->frame_prepare();

		auto commands = m_renderer->get_commands();

		for (i32 i = static_cast<i32>(m_layers.size() - 1); i >= 0; i--) {
			m_layers[static_cast<u32>(i)]->on_update(delta_time);
		}

		for (i32 i = static_cast<i32>(m_layers.size() - 1); i >= 0; i--) {
			m_layers[static_cast<u32>(i)]->on_render(commands);
		}

		m_renderer->submit(commands);
		m_renderer->present();

		m_renderer->frame_cleanup();

		// break;
	}

	toki::println("Stopping application");
}

void Engine::attach_layer(UniquePtr<Layer>&& layer) {
	layer->m_renderer = m_renderer.get();
	layer->m_window = m_window.get();
	layer->on_attach();
	m_layers.push_back(toki::move(layer));
}

void Engine::cleanup() {
	m_renderer.reset();
	m_window.reset();
}

}  // namespace toki
