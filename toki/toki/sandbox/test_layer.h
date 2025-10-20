#pragma once

#include <toki/runtime/runtime.h>

class TestLayer : public toki::Layer {
public:
	TestLayer() = delete;
	TestLayer(toki::f32 offset);

	virtual void on_attach() override;
	virtual void on_detach() override;
	virtual void on_update(toki::f32 delta_time) override;
	virtual void on_render(toki::Commands* commands) override;
	virtual void on_event(toki::Window* window, toki::Event& event) override;

private:
	toki::ShaderHandle m_shader;
	toki::ShaderLayoutHandle m_shaderLayout;
	toki::BufferHandle m_vertexBuffer;
	toki::BufferHandle m_indexBuffer;
	toki::BufferHandle m_uniformBuffer;
	toki::TextureHandle m_texture;
	toki::SamplerHandle m_sampler;

	toki::f32 m_imageScale = 1.0;
	toki::f32 m_offset = 1;
	toki::f32 m_color = 0;
	toki::Vector3 m_position{};
	toki::f32 m_directions[4];

	toki::Camera m_camera;
};
