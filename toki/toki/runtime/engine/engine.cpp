#include "toki/runtime/engine/engine.h"

namespace toki::runtime {

renderer::ShaderHandle shader;
renderer::ShaderLayoutHandle layout;

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

	renderer::ShaderLayoutConfig shader_layout_config{};
	layout = m_renderer->create_shader_layout(shader_layout_config);

	renderer::ColorFormat color_formats[1]{ renderer::ColorFormat::RGBA8 };

	renderer::ShaderConfig shader_config{};
	shader_config.color_formats = color_formats;
	shader_config.layout_handle = layout;
	shader_config.options.primitive_topology = renderer::PrimitiveTopology::TRIANGLE_LIST;
	shader_config.sources[renderer::ShaderStage::SHADER_STAGE_VERTEX] = R"(
		#version 450

		vec2 positions[3] = vec2[](
			vec2(0.0, -0.5),
			vec2(0.5, 0.5),
			vec2(-0.5, 0.5)
		);

		void main() {
			gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
		}
	)";
	shader_config.sources[renderer::ShaderStage::SHADER_STAGE_FRAGMENT] = R"(
		#version 450

		layout(location = 0) out vec4 outColor;

		void main() {
			outColor = vec4(1.0, 0.0, 0.0, 1.0);
		}
	)";
	shader = m_renderer->create_shader(shader_config);
}

Engine::~Engine() {
	m_renderer->destroy_handle(shader);
	m_renderer->destroy_handle(layout);

	cleanup();
}

void Engine::run() {
	toki::println("Starting application");

	while (m_running) {
		m_renderer->frame_prepare();

		auto commands = m_renderer->get_commands();
		commands->begin_pass();
		commands->bind_shader(shader);
		commands->draw(3);
		commands->end_pass();

		m_renderer->submit(commands);
		m_renderer->present();

		m_renderer->frame_cleanup();
	}
}

const platform::Window* Engine::get_window(u32 index) const {
	return m_window.get();
}

void Engine::cleanup() {
	m_renderer.reset();
	m_window.reset();
}

}  // namespace toki::runtime
