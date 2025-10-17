#include "test_layer.h"

struct Uniform {
	toki::Matrix4 model;
	toki::Matrix4 view;
	toki::Matrix4 projection;
	toki::Vector3 color;
};

TestLayer::TestLayer(toki::f32 offset): m_offset(offset) {}

void TestLayer::on_attach() {
	using namespace toki;
	using namespace toki;

	{
		DynamicArray<UniformConfig> set0_uniforms;
		// count, binding, type shader_stage_flags
		set0_uniforms.emplace_back(1, 0, UniformType::UNIFORM_BUFFER, ShaderStageFlags::SHADER_STAGE_VERTEX);

		DynamicArray<UniformConfig> set1_uniforms;
		set1_uniforms.emplace_back(1, 0, UniformType::TEXTURE_WITH_SAMPLER, ShaderStageFlags::SHADER_STAGE_FRAGMENT);

		DynamicArray<UniformSetConfig> uniform_set_configs;
		uniform_set_configs.emplace_back(set0_uniforms);
		uniform_set_configs.emplace_back(set1_uniforms);

		ShaderLayoutConfig shader_layout_config{};
		shader_layout_config.uniform_sets = uniform_set_configs;
		m_shaderLayout = m_renderer->create_shader_layout(shader_layout_config);
	}

	{
		ColorFormat color_formats[1]{ ColorFormat::RGBA8 };

		VertexBindingDescription bindings[] = { { 0, sizeof(f32) * 5, VertexInputRate::VERTEX } };
		VertexAttributeDescription attributes[] = {
			{ 0, 0, VertexFormat::FLOAT3, 0 },
			{ 1, 0, VertexFormat::FLOAT2, sizeof(toki::Vector3) },
		};

		ShaderConfig shader_config{};
		shader_config.color_formats = color_formats;
		shader_config.layout_handle = m_shaderLayout;
		shader_config.options.front_face = FrontFace::CLOCKWISE;
		shader_config.options.primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
		shader_config.options.polygon_mode = PolygonMode::FILL;
		shader_config.options.cull_mode = CullMode::NONE;
		shader_config.bindings = bindings;
		shader_config.attributes = attributes;
		shader_config.sources[ShaderStageFlags::SHADER_STAGE_VERTEX] = R"(
		#version 450

		layout(location = 0) out vec3 out_color;
		layout(location = 1) out vec2 out_uv;

		layout(location = 0) in vec3 in_position;
		layout(location = 1) in vec2 in_uv;

		layout(set = 0, binding = 0) uniform MyUniform {
			mat4 model;
			mat4 view;
			mat4 projection;
			vec3 color;
		} ubo;

		void main() {
			gl_Position = ubo.projection * ubo.view * ubo.model * vec4(in_position, 1.0);
			out_color = ubo.color;
			out_uv = in_uv;
		}
	)";
		shader_config.sources[ShaderStageFlags::SHADER_STAGE_FRAGMENT] = R"(
		#version 450

		layout(location = 0) in vec3 in_color;
		layout(location = 1) in vec2 in_uv;

		layout(location = 0) out vec4 out_color;

		layout(set = 1, binding = 0) uniform sampler2D tex_sampler;

		void main() {
			out_color = texture(tex_sampler, in_uv) * vec4(in_color, 1.0);
		}
	)";
		m_shader = m_renderer->create_shader(shader_config);
	}

	{
		struct Vertex {
			toki::Vector3 pos;
			toki::Vector2 uv;
		};

		toki::f32 offset = 200 * m_offset;
		Vertex vertices[] = {
			{ { offset, -offset, 0.0f }, { 1.0f, 0.0f } },	 // bottom-right
			{ { offset, offset, 0.0f }, { 1.0f, 1.0f } },	 // top-right
			{ { -offset, -offset, 0.0f }, { 0.0f, 0.0f } },	 // bottom-left
			{ { -offset, offset, 0.0f }, { 0.0f, 1.0f } },	 // top-left
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
		Uniform uniform{ {}, {}, {}, { 1.0, 1.0, 1.0 } };

		BufferConfig uniform_buffer_config{};
		uniform_buffer_config.size = sizeof(uniform);
		uniform_buffer_config.type = BufferType::UNIFORM;
		m_uniformBuffer = m_renderer->create_buffer(uniform_buffer_config);
		m_renderer->set_buffer_data(m_uniformBuffer, &uniform, sizeof(uniform));

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
		uint8_t pixels[] = { 80, 80, 80, 255, 255, 0, 255, 255, 255, 0, 255, 255, 80, 80, 80, 255 };

		TextureConfig texture_config{};
		texture_config.flags = TextureFlags::SAMPLED | TextureFlags::WRITABLE;
		texture_config.width = 2;
		texture_config.height = 2;
		m_texture = m_renderer->create_texture(texture_config);

		m_renderer->set_texture_data(m_texture, pixels, sizeof(pixels));
	}

	{
		SamplerConfig sampler_config{};
		sampler_config.use_normalized_coords = true;
		sampler_config.mag_filter = SamplerFilter::NEAREST;
		sampler_config.min_filter = SamplerFilter::NEAREST;
		m_sampler = m_renderer->create_sampler(sampler_config);
	}

	SetUniform texture_uniform{};
	texture_uniform.handle.texture_with_sampler.sampler = m_sampler;
	texture_uniform.handle.texture_with_sampler.texture = m_texture;
	texture_uniform.type = UniformType::TEXTURE_WITH_SAMPLER;
	texture_uniform.binding = 0;
	texture_uniform.array_element = 0;
	texture_uniform.set_index = 1;

	SetUniform uniforms_to_set[] = { texture_uniform };

	SetUniformConfig set_uniform_config{};
	set_uniform_config.layout = m_shaderLayout;
	set_uniform_config.uniforms = uniforms_to_set;
	m_renderer->set_uniforms(set_uniform_config);
}

void TestLayer::on_detach() {
	m_renderer->destroy_handle(m_indexBuffer);
	m_renderer->destroy_handle(m_vertexBuffer);
	m_renderer->destroy_handle(m_shader);
	m_renderer->destroy_handle(m_shaderLayout);
	m_renderer->destroy_handle(m_texture);
	m_renderer->destroy_handle(m_sampler);
}

void TestLayer::on_render(toki::Commands* cmd) {
	toki::BeginPassConfig begin_pass_config{};
	begin_pass_config.render_area_size = m_window->get_dimensions();

	cmd->begin_pass(begin_pass_config);
	cmd->bind_shader(m_shader);
	cmd->bind_index_buffer(m_indexBuffer);
	cmd->bind_vertex_buffer(m_vertexBuffer);
	cmd->bind_uniforms(m_shaderLayout);
	cmd->draw_indexed(6);
	cmd->end_pass();
}

void TestLayer::on_update(toki::f32 delta_time) {
	using namespace toki;
	using namespace toki;

	if (m_window->is_key_down(toki::Key::A)) {
		m_directions[0] = 1.0;
	} else {
		m_directions[0] = 0.0;
	}

	if (m_window->is_key_down(toki::Key::D)) {
		m_directions[1] = -1.0;
	} else {
		m_directions[1] = 0.0;
	}

	if (m_window->is_key_down(toki::Key::W)) {
		m_directions[2] = -1.0;
	} else {
		m_directions[2] = 0.0;
	}

	if (m_window->is_key_down(toki::Key::S)) {
		m_directions[3] = 1.0;
	} else {
		m_directions[3] = 0.0;
	}

	Vector3 direction{};
	direction.x = m_directions[0] + m_directions[1];
	direction.y = m_directions[2] + m_directions[3];

	m_color += delta_time * 0.2;
	if (m_color > 1.0f) {
		m_color = 0.0f;
	}

	Camera camera;
	camera.set_view(look_at({ 0, 0, -5 }, { 0, 0, 0 }, { 0, 1, 0 }));
	camera.set_projection(ortho(-400, 400, -300, 300, 0.01, 100.0));

	m_position += (direction * delta_time * 1000);

	constexpr Matrix4 model = Matrix4();
	Matrix4 a = model.translate(m_position).rotate(Vector3(0.0f, 0.0f, 1.0f), -m_color * TWO_PI);

	Uniform uniform{ a, camera.get_view(), camera.get_projection(), { m_color, m_color, m_color } };

	m_renderer->set_buffer_data(m_uniformBuffer, &uniform, sizeof(Uniform));

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

void TestLayer::on_event([[maybe_unused]] toki::Window* window, [[maybe_unused]] toki::Event& event) {
	if (event.type() == toki::EventType::KEY_PRESS && event.data().key.key == toki::Key::SPACE) {
		m_position = {};
	}
}
