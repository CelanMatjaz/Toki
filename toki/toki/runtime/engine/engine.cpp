#include "toki/runtime/engine/engine.h"
#include "toki/core/common/common.h"

namespace toki::runtime {

Engine::Engine(const EngineConfig& config): m_running(true) {
	TK_ASSERT(s_runtime == nullptr);
	s_runtime = this;

	platform::WindowConfig window_config{};
	window_config.title = "Window";
	window_config.width = 800;
	window_config.height = 600;
	window_config.flags = platform::SHOW_ON_CREATE;
	m_window = toki::make_unique<platform::Window>(window_config);

	renderer::RendererConfig renderer_config{};
	renderer_config.window = m_window.get();
	m_renderer = renderer::Renderer::create(renderer_config);
}

Engine::~Engine() {
	cleanup();
}

void Engine::run() {
	toki::println("Starting application");

	while (m_running) {
		m_renderer->frame_prepare();

		auto commands = m_renderer->get_commands();

		for (u32 i = 0; i < m_layers.size(); i++) {
			m_layers[i]->on_render(*commands);
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
	m_layers.emplace_back(toki::move(layer));
}


const platform::Window* Engine::get_window(u32 index) const {
	return m_window.get();
}

void Engine::cleanup() {
	m_renderer.reset();
	m_window.reset();
}

}  // namespace toki::runtime
