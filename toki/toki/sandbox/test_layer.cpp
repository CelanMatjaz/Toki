#include "test_layer.h"

void TestLayer::on_attach() {
	using namespace toki;
	using namespace toki::runtime;
	using namespace toki::renderer;

	{
		renderer::ShaderLayoutConfig shader_layout_config{};
		m_shaderLayout = m_renderer->create_shader_layout(shader_layout_config);
	}

	{
		renderer::ColorFormat color_formats[1]{ renderer::ColorFormat::RGBA8 };

		renderer::ShaderConfig shader_config{};
		shader_config.color_formats = color_formats;
		shader_config.layout_handle = m_shaderLayout;
		shader_config.options.front_face = renderer::FrontFace::CLOCKWISE;
		shader_config.options.primitive_topology = renderer::PrimitiveTopology::TRIANGLE_LIST;
		shader_config.options.polygon_mode = renderer::PolygonMode::FILL;
		shader_config.options.cull_mode = renderer::CullMode::NONE;
		shader_config.sources[renderer::ShaderStage::SHADER_STAGE_VERTEX] = R"(
		#version 450

		layout(location = 0) out vec3 fragColor;

		vec2 positions[3] = vec2[](
			vec2(0.0, -0.5),
			vec2(0.5, 0.5),
			vec2(-0.5, 0.5)
		);

		vec3 colors[3] = vec3[](
			vec3(1.0, 0.0, 0.0),
			vec3(0.0, 1.0, 0.0),
			vec3(0.0, 0.0, 1.0)
		);

		void main() {
			gl_Position = vec4(positions[gl_VertexIndex], 0.5, 1.0);
			fragColor = colors[gl_VertexIndex];
		}
	)";
		shader_config.sources[renderer::ShaderStage::SHADER_STAGE_FRAGMENT] = R"(
		#version 450

		layout(location = 0) in vec3 fragColor;

		layout(location = 0) out vec4 outColor;

		void main() {
			outColor = vec4(fragColor, 1.0);
		}
	)";
		m_shader = m_renderer->create_shader(shader_config);
	}

	{
		struct Vertex {
			toki::Vec3f32 pos;
		};

		toki::f32 offset = 0.5;
		Vertex vertices[] = {
			{ { offset, -offset, 0.0f } },
			{ { offset, offset, 0.0f } },
			{ { -offset, -offset, 0.0f } },
			{ { -offset, offset, 0.0f } },
		};

		BufferConfig vertex_buffer_config{};
		vertex_buffer_config.size = sizeof(vertices);
		vertex_buffer_config.type = BufferType::VERTEX;
		m_vertexBuffer = m_renderer->create_buffer(vertex_buffer_config);
		m_renderer->set_buffer_data(m_vertexBuffer, &vertices, sizeof(vertices));
	}

	{
		toki::u32 indices[] = { 0, 1, 2, 2, 3, 0 };

		BufferConfig index_buffer_config{};
		index_buffer_config.size = sizeof(indices);
		index_buffer_config.type = BufferType::VERTEX;
		m_indexBuffer = m_renderer->create_buffer(index_buffer_config);
		m_renderer->set_buffer_data(m_indexBuffer, &indices, sizeof(indices));
	}
}

void TestLayer::on_detach() {
	m_renderer->destroy_handle(m_indexBuffer);
	m_renderer->destroy_handle(m_vertexBuffer);
	m_renderer->destroy_handle(m_shader);
	m_renderer->destroy_handle(m_shaderLayout);
}

void TestLayer::on_render(toki::renderer::Commands* cmd) {
	cmd->begin_pass();
	cmd->bind_shader(m_shader);
	cmd->draw(3);
	cmd->end_pass();
}

void TestLayer::on_update(toki::f32 delta_time) {}
