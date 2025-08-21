#include "test_layer.h"

#include "toki/renderer_types.h"

void TestLayer::on_attach() {
	{
		toki::FramebufferConfig framebuffer_config{};
		framebuffer_config.color_format = toki::ColorFormat::RGBA8;
		framebuffer_config.image_width = 600;
		framebuffer_config.image_height = 600;
		framebuffer_config.color_format_count = 1;
		m_renderer->framebuffer_create(framebuffer_config);
	}

	{
		toki::ShaderConfig shader_config{};
		m_renderer->shader_create(m_framebuffer, shader_config);
	}

	{
		struct Vertex {
			toki::Vec3<toki::f32> pos;
		};

		toki::f32 offset = 0.5;
		Vertex vertices[] = {
			{ { offset, -offset, 0.0f } },
			{ { offset, offset, 0.0f } },
			{ { -offset, -offset, 0.0f } },
			{ { -offset, offset, 0.0f } },
		};

		toki::BufferConfig vertex_buffer_config{};
		vertex_buffer_config.size = sizeof(vertices);
		vertex_buffer_config.type = toki::BufferType::VERTEX;
		m_vertex_buffer = m_renderer->buffer_create(vertex_buffer_config);
		m_renderer->buffer_set_data(m_vertex_buffer, sizeof(vertices), &vertices);
	}

	{
		toki::u32 indices[] = { 0, 1, 2, 2, 3, 0 };

		toki::BufferConfig index_buffer_config{};
		index_buffer_config.size = sizeof(indices);
		index_buffer_config.type = toki::BufferType::VERTEX;
		m_index_buffer = m_renderer->buffer_create(index_buffer_config);
		m_renderer->buffer_set_data(m_index_buffer, sizeof(indices), &indices);
	}
}

void TestLayer::on_detach() {}

void TestLayer::on_render(toki::RendererCommands& cmd) {
	TK_LOG_ERROR("Render");
	cmd.begin_rendering(m_framebuffer, toki::Rect2D{ { 0, 0 }, { 600, 600 } });

	cmd.end_rendering();
}

void TestLayer::on_update(toki::f32 delta_time) {}

void TestLayer::submit_test(toki::RendererCommands& cmd) {
	cmd.begin_rendering(m_framebuffer, toki::Rect2D{ { 0, 0 }, { 600, 600 } });

	cmd.bind_shader(m_shader);
	cmd.bind_buffer(m_index_buffer);
	cmd.bind_buffer(m_vertex_buffer);
	cmd.draw_indexed(1);

	cmd.end_rendering();
}
