#include "toki/runtime/engine/engine.h"

namespace toki {

static volatile b8 run = true;

Engine::Engine([[maybe_unused]] const EngineConfig& config): m_running(true) {
	TK_LOG_INFO("Initializing engine");

	WindowConfig window_config{};
	window_config.title			 = "Window";
	window_config.dimensions	 = { 800, 600 };
	window_config.min_dimensions = { 400, 300 };
	window_config.flags			 = WINDOW_CREATE_FLAG_SHOW_ON_CREATE | WINDOW_CREATE_FLAG_RESIZABLE;
	m_window					 = toki::make_unique<Window>(window_config);

	RendererConfig renderer_config{};
	renderer_config.window = m_window.get();
	m_renderer			   = Renderer::create(renderer_config);

	SystemManagerConfig system_manager_config{};
	m_systemManager = SystemManager::create(system_manager_config);
}

Engine::~Engine() {
	cleanup();
}

void Engine::run() {
	TK_LOG_INFO("Starting application");

	Time now		   = Time::now();
	Time previous_time = now;
	while (m_running) {
		m_window->pre_poll_events();
		window_system_poll_events();

		EventQueue& event_queue = m_window->get_event_queue();
		for (Event& event : event_queue) {
			if (event.type() == EventType::NONE) {
				continue;
			}

			if (event.has_type(EventType::KEY_PRESS) && event.data().key.key == Key::ENTER) {
				TK_LOG_INFO("Pressed enter, stopping application");
				m_running = false;
			}

			for (i32 i = static_cast<i32>(m_layers.size() - 1); i >= 0; i--) {
				m_layers[static_cast<u32>(i)]->on_event(event);
				if (event.handled()) {
					break;
				}
			}
		}
		event_queue.clear();

		now			   = Time::now();
		f64 delta_time = (now - previous_time).as<TimePrecision::Seconds>();
		previous_time  = now;

		m_renderer->frame_prepare();

		for (i32 i = static_cast<i32>(m_layers.size() - 1); i >= 0; i--) {
			m_layers[static_cast<u32>(i)]->on_update(delta_time);
		}

		for (i32 i = static_cast<i32>(m_layers.size() - 1); i >= 0; i--) {
			m_layers[static_cast<u32>(i)]->on_render();
		}

		m_renderer->present();

		m_renderer->frame_cleanup();
	}

	TK_LOG_INFO("Stopping application");
}

void Engine::attach_layer(UniquePtr<Layer>&& layer) {
	TK_LOG_INFO("Attaching new layer");
	layer->m_engine = this;
	layer->on_attach();
	m_layers.emplace_back(toki::move(layer));
}

void Engine::cleanup() {
	m_renderer.reset();
	m_window.reset();
}

}  // namespace toki
