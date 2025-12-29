#include "toki/sandbox/test_font_layer.h"

using namespace toki;

struct Uniform {
	toki::Matrix4 model;
	toki::Matrix4 view;
	toki::Matrix4 projection;
	toki::Vector3 color;
};

void TestFontLayer::on_attach() {
	create_font_resources();
	create_font_shader_handle();
	create_font_shader();
	create_font_uniform();
}

void TestFontLayer::on_detach() {
	toki::Renderer* renderer = m_engine->renderer();

	m_fontGeometry.free(renderer);

	renderer->destroy_handle(m_sampler);
	renderer->destroy_handle(m_fontShader);
	renderer->destroy_handle(m_fontUniformBuffer);
	renderer->destroy_handle(m_fontShaderLayout);
}

void TestFontLayer::on_update(toki::f32 delta_time) {
	toki::Renderer* renderer = m_engine->renderer();

	if (m_textRotation > 360) {
		m_textRotation = 0;
	}

	m_textRotation += 30 * delta_time;

	Matrix4 model = Matrix4().rotate({ 1.0, 1.0, 0.0 }, toki::radians(m_textRotation)).translate({ 200.0, 200.0, 0.0 });
	Uniform uniform{ model, m_view2D, m_projection2D, { 0.0 } };
	renderer->set_buffer_data(m_fontUniformBuffer, &uniform, sizeof(Uniform));
}

void TestFontLayer::on_render() {
	m_engine->renderer()->submit([this](toki::Commands* cmd) {
		DynamicArray<RenderTarget> render_targets;
		render_targets.emplace_back(TextureHandle{}, RenderTargetLoadOp::LOAD, RenderTargetStoreOp::STORE);

		toki::BeginPassConfig begin_pass_config{};
		begin_pass_config.render_area_size		 = m_engine->window()->get_dimensions();
		begin_pass_config.swapchain_target_index = 0;
		begin_pass_config.render_targets		 = render_targets;

		cmd->begin_pass(begin_pass_config);
		cmd->bind_shader(m_fontShader);
		cmd->bind_uniforms(m_fontShaderLayout);
		m_fontGeometry.draw(cmd);
		cmd->end_pass();
	});
}

void TestFontLayer::on_event(toki::Event& event) {
	if (event.has_type(EventType::WINDOW_RESIZE)) {
		auto data	   = event.data().window;
		m_projection2D = toki::ortho(0, data.dimensions.x, data.dimensions.y, 0, 0.0001, 100.0);
	}
}

void TestFontLayer::create_font_resources() {
	toki::Renderer* renderer = m_engine->renderer();
	toki::Window* window	 = m_engine->window();

	toki::LoadFontConfig load_font_config{};
	load_font_config.size		= 50;
	load_font_config.atlas_size = { 512, 512 };
	load_font_config.path		= StringView("assets/fonts/RobotoMono-Regular.ttf");
	load_font_config.renderer	= renderer;

	const toki::String font_name = "Test font";
	m_engine->system_manager()->font_system().load_font(font_name, load_font_config);

	toki::FontSystem& font_system = m_engine->system_manager()->font_system();

	m_font		   = font_system.get_font(font_name);
	m_fontGeometry = font_system.generate_geometry(font_name, "This is a test");
	m_fontGeometry.upload(renderer);

	const Vector2u32 window_dimensions = window->get_dimensions();
	m_projection2D					   = toki::ortho(0, window_dimensions.x, window_dimensions.y, 0, 0.0001, 100.0);
	m_view2D						   = Matrix4();
}

void TestFontLayer::create_font_shader_handle() {
	DynamicArray<UniformConfig> set0_uniforms;
	set0_uniforms.emplace_back(1, 0, UniformType::UNIFORM_BUFFER, ShaderStageFlags::SHADER_STAGE_VERTEX);
	set0_uniforms.emplace_back(1, 1, UniformType::TEXTURE_WITH_SAMPLER, ShaderStageFlags::SHADER_STAGE_FRAGMENT);

	DynamicArray<UniformSetConfig> uniform_set_configs;
	uniform_set_configs.emplace_back(set0_uniforms);

	ShaderLayoutConfig shader_layout_config{};
	shader_layout_config.uniform_sets = uniform_set_configs;

	m_fontShaderLayout = m_engine->renderer()->create_shader_layout(shader_layout_config);
}

void TestFontLayer::create_font_shader() {
	ColorFormat color_formats[1]{ ColorFormat::RGBA8 };

	ResourceData vertex_shader	 = load_text("assets/shaders/font.glsl.vert");
	ResourceData fragment_shader = load_text("assets/shaders/font.glsl.frag");

	ShaderConfig shader_config{};
	shader_config.color_formats				 = color_formats;
	shader_config.depth_format				 = ColorFormat::DEPTH_STENCIL;
	shader_config.layout_handle				 = m_fontShaderLayout;
	shader_config.options.front_face		 = FrontFace::COUNTER_CLOCKWISE;
	shader_config.options.primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
	shader_config.options.polygon_mode		 = PolygonMode::FILL;
	shader_config.options.cull_mode			 = CullMode::NONE;
	shader_config.options.depth_test_enable	 = false;
	shader_config.options.depth_write_enable = true;
	shader_config.options.depth_compare_op	 = CompareOp::LESS;
	shader_config.options.enable_blending	 = true;
	shader_config.bindings					 = FontVertex::vertex_bindings;
	shader_config.attributes				 = FontVertex::vertex_attributes;
	shader_config.sources[ShaderStageFlags::SHADER_STAGE_VERTEX] =
		StringView(static_cast<char*>(vertex_shader.data), vertex_shader.size);
	shader_config.sources[ShaderStageFlags::SHADER_STAGE_FRAGMENT] =
		StringView(static_cast<char*>(fragment_shader.data), fragment_shader.size);
	m_fontShader = m_engine->renderer()->create_shader(shader_config);
}

void TestFontLayer::create_font_uniform() {
	toki::Renderer* renderer = m_engine->renderer();

	{
		SamplerConfig sampler_config{};
		sampler_config.use_normalized_coords = true;
		sampler_config.mag_filter			 = SamplerFilter::LINEAR;
		sampler_config.min_filter			 = SamplerFilter::LINEAR;

		m_sampler = renderer->create_sampler(sampler_config);
	}

	Uniform uniform{ {}, {}, {}, { 1.0, 1.0, 1.0 } };

	BufferConfig uniform_buffer_config{};
	uniform_buffer_config.size = sizeof(uniform);
	uniform_buffer_config.type = BufferType::UNIFORM;

	m_fontUniformBuffer = m_engine->renderer()->create_buffer(uniform_buffer_config);

	SetUniform set_uniform0{};
	set_uniform0.handle.uniform_buffer = m_fontUniformBuffer;
	set_uniform0.type				   = UniformType::UNIFORM_BUFFER;
	set_uniform0.binding			   = 0;
	set_uniform0.array_element		   = 0;
	set_uniform0.set_index			   = 0;

	SetUniform texture_uniform{};
	texture_uniform.handle.texture_with_sampler.sampler = m_sampler;
	texture_uniform.handle.texture_with_sampler.texture = m_font->atlas_handle;
	texture_uniform.type								= UniformType::TEXTURE_WITH_SAMPLER;
	texture_uniform.binding								= 1;
	texture_uniform.array_element						= 0;
	texture_uniform.set_index							= 0;

	SetUniform uniforms_to_set[] = { set_uniform0, texture_uniform };
	SetUniformConfig set_uniform_config{};
	set_uniform_config.layout	= m_fontShaderLayout;
	set_uniform_config.uniforms = uniforms_to_set;
	renderer->set_uniforms(set_uniform_config);
	renderer->set_buffer_data(m_fontUniformBuffer, &uniform, sizeof(uniform));
}
