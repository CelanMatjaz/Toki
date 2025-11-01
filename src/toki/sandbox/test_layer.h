#pragma once

#include <toki/runtime/runtime.h>

class TestLayer : public toki::Layer {
public:
	TestLayer() = default;

	virtual void on_attach() override;
	virtual void on_detach() override;
	virtual void on_update(toki::f32 delta_time) override;
	virtual void on_render(toki::Commands* commands) override;
	virtual void on_event(toki::Window* window, toki::Event& event) override;

	void create_shader();
	void create_model();
	void setup_textures();
	void create_depth_buffer();
	void create_font_resources();
	void setup_uniforms(
		const toki::ShaderLayoutHandle layout,
		const toki::TextureHandle texture,
		const toki::BufferHandle uniform_buffer);

private:
	toki::ShaderHandle m_shader;
	toki::ShaderLayoutHandle m_shaderLayout;
	toki::BufferHandle m_vertexBuffer;
	toki::BufferHandle m_indexBuffer;
	toki::BufferHandle m_uniformBuffer;
	toki::TextureHandle m_texture;
	toki::TextureHandle m_depthBuffer;
	toki::SamplerHandle m_sampler;

	toki::BufferHandle m_fontUniformBuffer;
	toki::ShaderLayoutHandle m_fontShaderLayout;
	toki::ShaderHandle m_fontShader;

	toki::u32 m_vertexCount = 0;
	toki::f32 m_imageScale	= 1.0;
	toki::f32 m_offset		= 1;
	toki::f32 m_color		= 0.5;

	toki::ConstWrapper<toki::Font> m_font;
	toki::Geometry m_fontGeometry;

	toki::FreeFlightCameraController m_cameraController;
	toki::Matrix4 m_view2D;
	toki::Matrix4 m_projection2D;
};
