#pragma once

#include <toki/core/core.h>
#include <toki/runtime/runtime.h>

class TestFontLayer : public toki::Layer {
public:
	virtual void on_attach() override;
	virtual void on_detach() override;
	virtual void on_update(toki::f32 delta_time) override;
	virtual void on_render() override;
	virtual void on_event(toki::Event& event) override;

	void create_font_resources();
	void create_font_shader_handle();
	void create_font_shader();
	void create_font_uniform();

private:
	toki::ShaderLayoutHandle m_fontShaderLayout;
	toki::ShaderHandle m_fontShader;
	toki::BufferHandle m_fontUniformBuffer;
	toki::SamplerHandle m_sampler;
	toki::ConstWrapper<toki::Font> m_font;
	toki::Geometry m_fontGeometry;

	toki::Matrix4 m_view2D;
	toki::Matrix4 m_projection2D;

	toki::f32 m_textRotation{};
};
