#pragma once

#include <toki/runtime/runtime.h>

class TestLayer : public toki::runtime::Layer {
public:
	TestLayer() = default;
	TestLayer(toki::f32 offset);

	virtual void on_attach() override;
	virtual void on_detach() override;
	virtual void on_update(toki::f32 delta_time) override;
	virtual void on_render(toki::renderer::Commands* commands) override;

private:
	toki::renderer::ShaderHandle m_shader;
	toki::renderer::ShaderLayoutHandle m_shaderLayout;
	toki::renderer::BufferHandle m_vertexBuffer;
	toki::renderer::BufferHandle m_indexBuffer;

	toki::f32 m_offset = 0;
};
