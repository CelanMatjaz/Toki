#include "test_layer.h"

using namespace toki;

struct Uniform {
	toki::Matrix4 model;
	toki::Matrix4 view;
	toki::Matrix4 projection;
	toki::Vector3 color;
};

void TestLayer::on_attach() {
	using namespace toki;

	toki::Renderer* renderer = m_engine->renderer();

	create_shader();
	create_model();

	// m_camera.set_projection(ortho(400, -400, -300, 300, 0.01, 10000.0));
	m_cameraController.camera().set_projection(
		perspective(toki::radians(90.0), 800 / static_cast<f32>(800), 0.01, 1000.0));
	// m_camera.set_view(look_at({ 0, 0, 0.5 }, { 0, 0, 0 }, { 0, 1, 0 }));
	m_cameraController.camera().set_position({ 0.5, 0.0, 0.5 });
	m_cameraController.camera().set_rotation({ 0.0, 0.0, 0.0 });

	setup_textures();

	Uniform uniform{ {}, {}, {}, { 1.0, 1.0, 1.0 } };
	BufferConfig uniform_buffer_config{};
	uniform_buffer_config.size = sizeof(uniform);
	uniform_buffer_config.type = BufferType::UNIFORM;
	m_uniformBuffer			   = renderer->create_buffer(uniform_buffer_config);
	renderer->set_buffer_data(m_uniformBuffer, &uniform, sizeof(uniform));

	setup_uniforms(m_shaderLayout, m_texture, m_uniformBuffer);
}

void TestLayer::on_detach() {
	toki::Renderer* renderer = m_engine->renderer();

	renderer->destroy_handle(m_indexBuffer);
	renderer->destroy_handle(m_vertexBuffer);
	renderer->destroy_handle(m_shader);
	renderer->destroy_handle(m_shaderLayout);
	renderer->destroy_handle(m_texture);
	renderer->destroy_handle(m_sampler);
}

void TestLayer::on_render(toki::Commands* cmd) {
	DynamicArray<RenderTarget> render_targets;
	render_targets.emplace_back(TextureHandle{}, RenderTargetLoadOp::CLEAR, RenderTargetStoreOp::STORE);
	RenderTarget depth_buffer{ m_depthBuffer, RenderTargetLoadOp::CLEAR, RenderTargetStoreOp::STORE };

	toki::BeginPassConfig begin_pass_config{};
	begin_pass_config.render_area_size		 = m_engine->window()->get_dimensions();
	begin_pass_config.depth_buffer			 = depth_buffer;
	begin_pass_config.swapchain_target_index = 0;
	begin_pass_config.render_targets = render_targets;

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

	toki::Renderer* renderer = m_engine->renderer();

	m_textRotation += 30 * delta_time;
	if (m_textRotation > 360) {
		m_textRotation = 0;
	}

	m_cameraController.on_update(delta_time, m_engine->window());

	Matrix4 model = Matrix4().rotate(Vector3(0.0f, 1.0f, 0.0f), -m_color * TWO_PI).scale(m_imageScale * 0.2).scale(10);

	Uniform uniform{ model,
					 m_cameraController.camera().get_view(),
					 m_cameraController.camera().get_projection(),
					 { m_color, m_color, m_color } };

	renderer->set_buffer_data(m_uniformBuffer, &uniform, sizeof(Uniform));

	SetUniform set_uniform{};
	set_uniform.handle.uniform_buffer = m_uniformBuffer;
	set_uniform.type				  = UniformType::UNIFORM_BUFFER;
	set_uniform.binding				  = 0;
	set_uniform.array_element		  = 0;
	set_uniform.set_index			  = 0;

	SetUniform uniforms_to_set[] = { set_uniform };

	SetUniformConfig set_uniform_config{};
	set_uniform_config.layout	= m_shaderLayout;
	set_uniform_config.uniforms = uniforms_to_set;
	renderer->set_uniforms(set_uniform_config);
}

void TestLayer::on_event([[maybe_unused]] toki::Window* window, [[maybe_unused]] toki::Event& event) {
	using namespace toki;

	m_cameraController.on_event(event);

	if (event.has_type(EventType::WINDOW_RESIZE)) {
		auto data = event.data().window;
		// toki::f32 half_width = dimensions.x / 2.0f;
		// toki::f32 half_height = dimensions.y / 2.0f;
		// m_camera.set_projection(ortho(half_width, -half_width, -half_height, half_height, 0.01, 100.0));
		m_cameraController.camera().set_projection(
			perspective(toki::radians(90.0), data.dimensions.x / static_cast<f32>(data.dimensions.y), 0.01, 1000.0));

		create_depth_buffer();
	}

	else if (event.has_type(EventType::MOUSE_SCROLL)) {
		m_imageScale += event.data().mouse.y * 0.2f;
	}
}

void TestLayer::create_shader() {
	toki::Renderer* renderer = m_engine->renderer();

	{
		DynamicArray<UniformConfig> set0_uniforms;
		// count, binding, type shader_stage_flags
		set0_uniforms.emplace_back(1, 0, UniformType::UNIFORM_BUFFER, ShaderStageFlags::SHADER_STAGE_VERTEX);
		set0_uniforms.emplace_back(1, 1, UniformType::TEXTURE_WITH_SAMPLER, ShaderStageFlags::SHADER_STAGE_FRAGMENT);

		DynamicArray<UniformSetConfig> uniform_set_configs;
		uniform_set_configs.emplace_back(set0_uniforms);

		ShaderLayoutConfig shader_layout_config{};
		shader_layout_config.uniform_sets = uniform_set_configs;

		m_shaderLayout = renderer->create_shader_layout(shader_layout_config);
	}

	ColorFormat color_formats[1]{ ColorFormat::RGBA8 };

	ResourceData vertex_shader	 = load_text("assets/shaders/test.glsl.vert");
	ResourceData fragment_shader = load_text("assets/shaders/test.glsl.frag");

	ShaderConfig shader_config{};
	shader_config.color_formats				 = color_formats;
	shader_config.depth_format				 = ColorFormat::DEPTH_STENCIL;
	shader_config.layout_handle				 = m_shaderLayout;
	shader_config.options.front_face		 = FrontFace::COUNTER_CLOCKWISE;
	shader_config.options.primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
	shader_config.options.polygon_mode		 = PolygonMode::FILL;
	shader_config.options.cull_mode			 = CullMode::BACK;
	shader_config.options.depth_test_enable	 = true;
	shader_config.options.depth_write_enable = true;
	shader_config.options.depth_compare_op	 = CompareOp::LESS;
	shader_config.bindings					 = Vertex::vertex_bindings;
	shader_config.attributes				 = Vertex::vertex_attributes;
	shader_config.sources[ShaderStageFlags::SHADER_STAGE_VERTEX] =
		StringView(static_cast<char*>(vertex_shader.data), vertex_shader.size);
	shader_config.sources[ShaderStageFlags::SHADER_STAGE_FRAGMENT] =
		StringView(static_cast<char*>(fragment_shader.data), fragment_shader.size);
	m_shader = renderer->create_shader(shader_config);
}

void TestLayer::create_model() {
	toki::Renderer* renderer = m_engine->renderer();

	auto model_data = toki::load_obj("assets/models/rabbit.obj");
	m_vertexCount	= model_data.index_data.size();

	BufferConfig vertex_buffer_config{};
	vertex_buffer_config.size = model_data.vertex_data.size() * sizeof(Vertex);
	vertex_buffer_config.type = BufferType::VERTEX;
	m_vertexBuffer			  = renderer->create_buffer(vertex_buffer_config);
	renderer->set_buffer_data(m_vertexBuffer, model_data.vertex_data.data(), vertex_buffer_config.size);

	BufferConfig index_buffer_config{};
	index_buffer_config.size = model_data.index_data.size() * sizeof(u32);
	index_buffer_config.type = BufferType::INDEX;
	m_indexBuffer			 = renderer->create_buffer(index_buffer_config);
	renderer->set_buffer_data(m_indexBuffer, model_data.index_data.data(), index_buffer_config.size);
}

void TestLayer::setup_uniforms(
	const ShaderLayoutHandle layout, const TextureHandle texture, const BufferHandle uniform_buffer) {
	toki::Renderer* renderer = m_engine->renderer();

	Uniform uniform{ {}, {}, {}, { 1.0, 1.0, 1.0 } };

	SetUniform set_uniform0{};
	set_uniform0.handle.uniform_buffer = uniform_buffer;
	set_uniform0.type				   = UniformType::UNIFORM_BUFFER;
	set_uniform0.binding			   = 0;
	set_uniform0.array_element		   = 0;
	set_uniform0.set_index			   = 0;

	SetUniform texture_uniform{};
	texture_uniform.handle.texture_with_sampler.sampler = m_sampler;
	texture_uniform.handle.texture_with_sampler.texture = texture;
	texture_uniform.type								= UniformType::TEXTURE_WITH_SAMPLER;
	texture_uniform.binding								= 1;
	texture_uniform.array_element						= 0;
	texture_uniform.set_index							= 0;

	SetUniform uniforms_to_set[] = { set_uniform0, texture_uniform };
	SetUniformConfig set_uniform_config{};
	set_uniform_config.layout	= layout;
	set_uniform_config.uniforms = uniforms_to_set;
	renderer->set_uniforms(set_uniform_config);
}

void TestLayer::setup_textures() {
	toki::Renderer* m_renderer = m_engine->renderer();

	{
		Resource texture_resource("assets/textures/pepe.jpg", ResourceType::TEXTURE);
		auto& metadata = texture_resource.metadata().texture;

		TextureConfig texture_config{};
		texture_config.flags	= TextureFlags::SAMPLED | TextureFlags::WRITABLE;
		texture_config.width	= metadata.width;
		texture_config.height	= metadata.height;
		texture_config.channels = metadata.channels;
		texture_config.format	= ColorFormat::RGBA8;
		m_texture				= m_renderer->create_texture(texture_config);

		m_renderer->set_texture_data(m_texture, texture_resource.data(), texture_resource.size());
	}

	{
		SamplerConfig sampler_config{};
		sampler_config.use_normalized_coords = true;
		sampler_config.mag_filter			 = SamplerFilter::LINEAR;
		sampler_config.min_filter			 = SamplerFilter::LINEAR;
		m_sampler							 = m_renderer->create_sampler(sampler_config);
	}

	create_depth_buffer();
}

void TestLayer::create_depth_buffer() {
	toki::Renderer* renderer = m_engine->renderer();

	if (m_depthBuffer.valid()) {
		renderer->destroy_handle(m_depthBuffer);
		m_depthBuffer = {};
	}

	auto window_dimensions = m_engine->window()->get_dimensions();
	TextureConfig texture_config{};
	texture_config.flags	= TextureFlags::DEPTH_STENCIL_ATTACHMENT | TextureFlags::WRITABLE;
	texture_config.width	= window_dimensions.x;
	texture_config.height	= window_dimensions.y;
	texture_config.channels = 1;
	texture_config.format	= ColorFormat::DEPTH_STENCIL;
	m_depthBuffer			= renderer->create_texture(texture_config);
}
