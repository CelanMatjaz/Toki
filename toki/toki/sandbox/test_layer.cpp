#include "test_layer.h"

#include "toki/core/containers/dynamic_array.h"
#include "toki/core/string/span.h"

TestLayer::TestLayer(toki::f32 offset): m_offset(offset) {}

void TestLayer::on_attach() {
	using namespace toki;
	using namespace toki::runtime;
	using namespace toki::renderer;

	{
		DynamicArray<UniformConfig> set0_uniforms;
		// count, binding, type shader_stage_flags
		set0_uniforms.emplace_back(1, 0, UniformType::UNIFORM_BUFFER, ShaderStageFlags::SHADER_STAGE_FRAGMENT);

		DynamicArray<UniformSetConfig> uniform_set_configs;
		uniform_set_configs.emplace_back(set0_uniforms);

		renderer::ShaderLayoutConfig shader_layout_config{};
		shader_layout_config.uniform_sets = uniform_set_configs;
		m_shaderLayout = m_renderer->create_shader_layout(shader_layout_config);
	}

	{
		renderer::ColorFormat color_formats[1]{ renderer::ColorFormat::RGBA8 };

		VertexBindingDescription bindings[] = { { 0, sizeof(f32) * 3, VertexInputRate::VERTEX } };
		VertexAttributeDescription attributes[] = { { 0, 0, VertexFormat::FLOAT3, 0 } };

		renderer::ShaderConfig shader_config{};
		shader_config.color_formats = color_formats;
		shader_config.layout_handle = m_shaderLayout;
		shader_config.options.front_face = renderer::FrontFace::CLOCKWISE;
		shader_config.options.primitive_topology = renderer::PrimitiveTopology::TRIANGLE_LIST;
		shader_config.options.polygon_mode = renderer::PolygonMode::FILL;
		shader_config.options.cull_mode = renderer::CullMode::NONE;
		shader_config.bindings = bindings;
		shader_config.attributes = attributes;
		shader_config.sources[renderer::ShaderStageFlags::SHADER_STAGE_VERTEX] = R"(
		#version 450

		layout(location = 0) out vec3 out_color;

		layout(location = 0) in vec3 in_position;

		vec3 colors[3] = vec3[](
			vec3(1.0, 0.0, 0.0),
			vec3(0.0, 1.0, 0.0),
			vec3(0.0, 0.0, 1.0)
		);

		void main() {
			gl_Position = vec4(in_position, 1.0);
			out_color = colors[gl_VertexIndex];
		}
	)";
		shader_config.sources[renderer::ShaderStageFlags::SHADER_STAGE_FRAGMENT] = R"(
		#version 450

		layout(location = 0) in vec3 in_color;

		layout(location = 0) out vec4 out_color;

		layout(set = 0, binding = 0) uniform MyUniform {
			vec3 color;
		} ubo;

		void main() {
			out_color = vec4(in_color + ubo.color, 1.0);
		}
	)";
		m_shader = m_renderer->create_shader(shader_config);
	}

	{
		struct Vertex {
			toki::Vec3f32 pos;
		};

		toki::f32 offset = 0.5f + m_offset;
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
		toki::u32 indices[] = { 0, 1, 2, 3, 1, 2 };

		BufferConfig index_buffer_config{};
		index_buffer_config.size = sizeof(indices);
		index_buffer_config.type = BufferType::INDEX;
		m_indexBuffer = m_renderer->create_buffer(index_buffer_config);
		m_renderer->set_buffer_data(m_indexBuffer, &indices, sizeof(indices));
	}

	{
		struct Color {
			f32 r, g, b;
		};

		Color color{ 1.0, 1.0, 1.0 };

		BufferConfig uniform_buffer_config{};
		uniform_buffer_config.size = sizeof(Color);
		uniform_buffer_config.type = BufferType::UNIFORM;
		m_uniformBuffer = m_renderer->create_buffer(uniform_buffer_config);
		m_renderer->set_buffer_data(m_uniformBuffer, &color, sizeof(Color));

		SetUniform set_uniform{};
		set_uniform.handle.uniform_buffer = m_uniformBuffer;
		set_uniform.type = UniformType::UNIFORM_BUFFER;
		set_uniform.binding = 0;
		set_uniform.array_element = 0;
		set_uniform.set_index = 0;

		SetUniform uniforms_to_set[] = { set_uniform };

		SetUniformConfig set_uniform_config{};
		set_uniform_config.layout = m_shaderLayout;
		set_uniform_config.uniforms = uniforms_to_set;
		m_renderer->set_uniforms(set_uniform_config);
	}

	{
		SamplerConfig sampler_config{};
		m_sampler = m_renderer->create_sampler(sampler_config);
	}
}

void TestLayer::on_detach() {
	m_renderer->destroy_handle(m_indexBuffer);
	m_renderer->destroy_handle(m_vertexBuffer);
	m_renderer->destroy_handle(m_shader);
	m_renderer->destroy_handle(m_shaderLayout);
	m_renderer->destroy_handle(m_texture);
	m_renderer->destroy_handle(m_sampler);
}

void TestLayer::on_render(toki::renderer::Commands* cmd) {
	cmd->begin_pass();
	cmd->bind_shader(m_shader);
	cmd->bind_index_buffer(m_indexBuffer);
	cmd->bind_vertex_buffer(m_vertexBuffer);
	cmd->bind_uniforms(m_shaderLayout);
	cmd->draw_indexed(6);
	cmd->end_pass();
}

void TestLayer::on_update(toki::f32 delta_time) {
	using namespace toki;
	using namespace toki::renderer;

	m_color += delta_time * 0.2;
	if (m_color > 1.0f) {
		m_color -= 1.0f;
	}

	struct Color {
		f32 r, g, b;
	};

	Color color{ m_color, m_color, m_color };

	m_renderer->set_buffer_data(m_uniformBuffer, &color, sizeof(Color));

	SetUniform set_uniform{};
	set_uniform.handle.uniform_buffer = m_uniformBuffer;
	set_uniform.type = UniformType::UNIFORM_BUFFER;
	set_uniform.binding = 0;
	set_uniform.array_element = 0;
	set_uniform.set_index = 0;

	SetUniform uniforms_to_set[] = { set_uniform };

	SetUniformConfig set_uniform_config{};
	set_uniform_config.layout = m_shaderLayout;
	set_uniform_config.uniforms = uniforms_to_set;
	m_renderer->set_uniforms(set_uniform_config);
}
