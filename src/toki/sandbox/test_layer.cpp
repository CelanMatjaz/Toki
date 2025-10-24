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

		VertexBindingDescription bindings[] = { { 0, sizeof(Vertex), VertexInputRate::VERTEX } };
		VertexAttributeDescription attributes[] = {
			{ 0, 0, VertexFormat::FLOAT3, 0 },
			{ 1, 0, VertexFormat::FLOAT3, sizeof(toki::Vector3) },
			{ 2, 0, VertexFormat::FLOAT2, 2 * sizeof(toki::Vector3) },
		};

		ShaderConfig shader_config{};
		shader_config.color_formats = color_formats;
		shader_config.depth_format = ColorFormat::DEPTH_STENCIL;
		shader_config.layout_handle = m_shaderLayout;
		shader_config.options.front_face = FrontFace::CLOCKWISE;
		shader_config.options.primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
		shader_config.options.polygon_mode = PolygonMode::FILL;
		shader_config.options.cull_mode = CullMode::NONE;
		shader_config.options.depth_test_enable = true;
		shader_config.options.depth_write_enable = true;
		shader_config.options.depth_compare_op = CompareOp::LESS;
		shader_config.bindings = bindings;
		shader_config.attributes = attributes;
		shader_config.sources[ShaderStageFlags::SHADER_STAGE_VERTEX] = R"(#version 450

		layout(location = 0) out vec3 out_color;
		layout(location = 1) out vec3 out_normals;
		layout(location = 2) out vec2 out_uv;
		layout(location = 3) out vec3 out_position;

		layout(location = 0) in vec3 in_position;
		layout(location = 1) in vec3 in_normals;
		layout(location = 2) in vec2 in_uv;

		layout(set = 0, binding = 0) uniform MyUniform {
			mat4 model;
			mat4 view;
			mat4 projection;
			vec3 color;
		} ubo;

		void main() {
			out_color = ubo.color;
			out_normals = normalize(mat3(ubo.model) * in_normals);
			out_uv = in_uv;
			vec4 world_pos = ubo.model * vec4(in_position, 1.0);
			out_position = world_pos.xyz;
			gl_Position = ubo.projection * ubo.view * ubo.model * vec4(in_position, 1.0);
		}
	)";
		shader_config.sources[ShaderStageFlags::SHADER_STAGE_FRAGMENT] = R"(
		#version 450

		layout(location = 0) in vec3 in_color;
		layout(location = 1) in vec3 in_normals;
		layout(location = 2) in vec2 in_uv;
		layout(location = 3) in vec3 in_position;

		layout(location = 0) out vec4 out_color;

		layout(set = 1, binding = 0) uniform sampler2D tex_sampler;

		vec3 light_pos = vec3(1.0, 2.0, 3.0);
		vec3 light_color = vec3(0.4, 0.8, 0.2);
		vec3 object_color = vec3(1.0, 1.0, 1.0);

		void main() {
			vec3 light_dir = normalize(light_pos - in_position);

			float diff = max(dot(normalize(in_normals), light_dir), 0.0);
			vec3 diffuse = diff * light_color;
			vec3 color = diffuse * vec3(texture(tex_sampler, in_uv));

			out_color = vec4(color, 1.0);
		}
	)";
		m_shader = m_renderer->create_shader(shader_config);
	}

	{
		auto model_data = toki::load_obj("rabbit.obj");
		m_vertexCount = model_data.index_data.size();

		BufferConfig vertex_buffer_config{};
		vertex_buffer_config.size = model_data.vertex_data.size() * sizeof(Vertex);
		vertex_buffer_config.type = BufferType::VERTEX;
		m_vertexBuffer = m_renderer->create_buffer(vertex_buffer_config);
		m_renderer->set_buffer_data(m_vertexBuffer, model_data.vertex_data.data(), vertex_buffer_config.size);

		BufferConfig index_buffer_config{};
		index_buffer_config.size = model_data.index_data.size() * sizeof(u32);
		index_buffer_config.type = BufferType::INDEX;
		m_indexBuffer = m_renderer->create_buffer(index_buffer_config);
		m_renderer->set_buffer_data(m_indexBuffer, model_data.index_data.data(), index_buffer_config.size);
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
		// uint8_t pixels[] = { 80, 80, 80, 255, 255, 0, 255, 255, 255, 0, 255, 255, 80, 80, 80, 255 };

		Resource texture_resource("pepe.jpg", ResourceType::TEXTURE);
		auto& metadata = texture_resource.metadata().texture;

		TextureConfig texture_config{};
		texture_config.flags = TextureFlags::SAMPLED | TextureFlags::WRITABLE;
		texture_config.width = metadata.width;
		texture_config.height = metadata.height;
		texture_config.channels = metadata.channels;
		texture_config.format = ColorFormat::RGBA8;
		m_texture = m_renderer->create_texture(texture_config);

		m_renderer->set_texture_data(m_texture, texture_resource.data(), texture_resource.size());
	}

	{
		SamplerConfig sampler_config{};
		sampler_config.use_normalized_coords = true;
		sampler_config.mag_filter = SamplerFilter::NEAREST;
		sampler_config.min_filter = SamplerFilter::NEAREST;
		m_sampler = m_renderer->create_sampler(sampler_config);
	}

	{
		TextureConfig texture_config{};
		texture_config.flags = TextureFlags::DEPTH_STENCIL_ATTACHMENT | TextureFlags::WRITABLE;
		texture_config.width = 800;
		texture_config.height = 600;
		texture_config.channels = 1;
		texture_config.format = ColorFormat::DEPTH_STENCIL;
		m_depthBuffer = m_renderer->create_texture(texture_config);
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

	// m_camera.set_projection(ortho(400, -400, -300, 300, 0.01, 10000.0));
	m_camera.set_projection(
		perspective(toki::convert_angle_to<AngleUnits::Radians>(90.0), 800 / static_cast<f32>(800), 0.01, 1000.0));
	m_camera.set_view(look_at({ 0, 0, 0.5 }, { 0, 0, 0 }, { 0, 1, 0 }));
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
	begin_pass_config.depth_buffer = m_depthBuffer;
	begin_pass_config.swapchain_target_index = 0;


	cmd->begin_pass(begin_pass_config);
	cmd->bind_shader(m_shader);
	cmd->bind_index_buffer(m_indexBuffer);
	cmd->bind_vertex_buffer(m_vertexBuffer);
	cmd->bind_uniforms(m_shaderLayout);
	cmd->draw_indexed(m_vertexCount);
	cmd->end_pass();
}

void TestLayer::on_update(toki::f32 delta_time) {
	using namespace toki;

	if (m_window->is_key_down(toki::Key::LEFT)) {
		m_directions[0] = -1.0;
	} else {
		m_directions[0] = 0.0;
	}

	if (m_window->is_key_down(toki::Key::RIGHT)) {
		m_directions[1] = 1.0;
	} else {
		m_directions[1] = 0.0;
	}

	if (m_window->is_key_down(toki::Key::UP)) {
		m_directions[2] = -1.0;
	} else {
		m_directions[2] = 0.0;
	}

	if (m_window->is_key_down(toki::Key::DOWN)) {
		m_directions[3] = 1.0;
	} else {
		m_directions[3] = 0.0;
	}

	if (m_window->is_key_down(toki::Key::W)) {
		m_directions[4] = -1.0;
	} else {
		m_directions[4] = 0.0;
	}

	if (m_window->is_key_down(toki::Key::S)) {
		m_directions[5] = 1.0;
	} else {
		m_directions[5] = 0.0;
	}

	Vector3 direction{};
	direction.x = m_directions[0] + m_directions[1];
	direction.y = m_directions[2] + m_directions[3];
	direction.z = m_directions[4] + m_directions[5];

	m_color += delta_time * 0.2;
	if (m_color > 1.0f) {
		m_color = 0.0f;
	}

	m_position += (direction * delta_time * .5);

	constexpr Matrix4 model = Matrix4();
	Matrix4 a = model.rotate(Vector3(0.0f, 1.0f, 0.0f), -m_color * TWO_PI).scale(m_imageScale * 0.2).scale(10);

	m_camera.set_view(look_at(m_position, { 0, 0, 0 }, { 0, 1, 0 }));
	Uniform uniform{ a, m_camera.get_view(), m_camera.get_projection(), { m_color, m_color, m_color } };

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
	using namespace toki;

	if (event.has_type(EventType::KEY_PRESS) && event.data().key.key == Key::SPACE) {
		m_position = {};
	}

	else if (event.has_type(EventType::WINDOW_RESIZE)) {
		auto dimensions = event.data().window;
		toki::f32 half_width = dimensions.x / 2.0f;
		toki::f32 half_height = dimensions.y / 2.0f;

		// m_camera.set_projection(ortho(half_width, -half_width, -half_height, half_height, 0.01, 100.0));
		m_camera.set_projection(perspective(
			toki::convert_angle_to<AngleUnits::Radians>(90.0),
			dimensions.x / static_cast<f32>(dimensions.y),
			0.01,
			1000.0));
	}

	else if (event.has_type(EventType::MOUSE_SCROLL)) {
		m_imageScale += event.data().mouse.y * 0.2f;
	}
}
