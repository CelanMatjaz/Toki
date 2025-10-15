#include "toki/runtime/engine/engine.h"

namespace toki::runtime {

Engine::Engine([[maybe_unused]] const EngineConfig& config): m_running(true) {
	WindowConfig window_config{};
	window_config.title = "Window";
	window_config.width = 800;
	window_config.height = 600;
	window_config.flags = WINDOW_FLAG_SHOW_ON_CREATE;
	m_window = toki::make_unique<Window>(window_config);

	renderer::RendererConfig renderer_config{};
	renderer_config.window = m_window.get();
	m_renderer = renderer::Renderer::create(renderer_config);
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

		Event event;
		while (m_window->poll_event(event)) {
			if (event.type() == EventType::MOUSE_MOVE) {
				toki::println("{} {}", event.data().window.x, event.data().window.y);
			}
		}

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
	layer->on_attach();
	m_layers.push_back(toki::move(layer));
}

void Engine::cleanup() {
	m_renderer.reset();
	m_window.reset();
}

}  // namespace toki::runtime
