#pragma once

#include <toki/runtime/runtime.h>

class TestLayer : public toki::Layer {
public:
	TestLayer() = delete;
	TestLayer(toki::f32 offset);

	virtual void on_attach() override;
	virtual void on_detach() override;
	virtual void on_update(toki::f32 delta_time) override;
	virtual void on_render(toki::renderer::Commands* commands) override;
	virtual void on_event(toki::Window* window, toki::Event& event) override;

private:
	toki::renderer::ShaderHandle m_shader;
	toki::renderer::ShaderLayoutHandle m_shaderLayout;
	toki::renderer::BufferHandle m_vertexBuffer;
	toki::renderer::BufferHandle m_indexBuffer;
	toki::renderer::BufferHandle m_uniformBuffer;
	toki::renderer::TextureHandle m_texture;
	toki::renderer::SamplerHandle m_sampler;

	toki::f32 m_offset = 1;
	toki::f32 m_color = 0;
	toki::Vector3 m_direction{};
	toki::f32 m_directions[4];
};
