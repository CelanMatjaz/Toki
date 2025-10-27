#include "test_layer.h"

using namespace toki;

struct Uniform {
	toki::Matrix4 model;
	toki::Matrix4 view;
	toki::Matrix4 projection;
	toki::Vector3 color;
};

TestLayer::TestLayer(toki::f32 offset): m_offset(offset) {}

void TestLayer::on_attach() {
	using namespace toki;

	create_shader();
	create_model();
	setup_textures();
	setup_uniforms();

	// m_camera.set_projection(ortho(400, -400, -300, 300, 0.01, 10000.0));
	m_camera.set_projection(
		perspective(toki::convert_angle_to<AngleUnits::Radians>(90.0), 800 / static_cast<f32>(800), 0.01, 1000.0));
	// m_camera.set_view(look_at({ 0, 0, 0.5 }, { 0, 0, 0 }, { 0, 1, 0 }));
	m_camera.set_position({ 0, 0, 0.5 });
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

	m_strafeDirection = 0;
	if (m_window->is_key_down(toki::Key::A)) {
		m_strafeDirection += -1;
	}

	if (m_window->is_key_down(toki::Key::D)) {
		m_strafeDirection += +1;
	}

	m_color += delta_time * 0.2;
	if (m_color > 1.0f) {
		m_color = 0.0f;
	}

	f32 sensitivity = 0.5;

	if (m_mouseDown) {
		Vector2 mouse_delta = m_window->get_mouse_delta();
		m_cameraRotation.x += mouse_delta.y * sensitivity;
		m_cameraRotation.y += mouse_delta.x * sensitivity;

		if (m_cameraRotation.x > 89.0f) {
			m_cameraRotation.x = 89.0f;
		}
		if (m_cameraRotation.x < -89.0f) {
			m_cameraRotation.x = -89.0f;
		}

		m_camera.set_rotation(m_cameraRotation);

		toki::println(Formatter<Vector3>::format(m_cameraRotation));
	}

	Matrix4 model = Matrix4().rotate(Vector3(0.0f, 1.0f, 0.0f), -m_color * TWO_PI).scale(m_imageScale * 0.2).scale(10);

	m_camera.set_view(look_at(m_position, { 0, 0, 0 }, { 0, 1, 0 }));
	Uniform uniform{ model, m_camera.get_view(), m_camera.get_projection(), { m_color, m_color, m_color } };

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

	else if (event.has_type(EventType::MOUSE_PRESS)) {
		m_mouseDown = true;
	}

	else if (event.has_type(EventType::MOUSE_RELEASE)) {
		m_mouseDown = false;
	}
}

void TestLayer::create_shader() {
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

	ColorFormat color_formats[1]{ ColorFormat::RGBA8 };

	VertexBindingDescription bindings[] = { { 0, sizeof(Vertex), VertexInputRate::VERTEX } };
	VertexAttributeDescription attributes[] = {
		{ 0, 0, VertexFormat::FLOAT3, 0 },
		{ 1, 0, VertexFormat::FLOAT3, sizeof(toki::Vector3) },
		{ 2, 0, VertexFormat::FLOAT2, 2 * sizeof(toki::Vector3) },
	};

	ResourceData vertex_shader = load_text("src/toki/sandbox/assets/shaders/test.glsl.vert");
	ResourceData fragment_shader = load_text("src/toki/sandbox/assets/shaders/test.glsl.frag");

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
	shader_config.sources[ShaderStageFlags::SHADER_STAGE_VERTEX] =
		StringView(static_cast<char*>(vertex_shader.data), vertex_shader.size);
	shader_config.sources[ShaderStageFlags::SHADER_STAGE_FRAGMENT] =
		StringView(static_cast<char*>(fragment_shader.data), fragment_shader.size);
	m_shader = m_renderer->create_shader(shader_config);
}

void TestLayer::create_model() {
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

void TestLayer::setup_uniforms() {
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
}

void TestLayer::setup_textures() {
	{
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
}
