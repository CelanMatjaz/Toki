#pragma once

#include <toki/engine.h>
#include <toki/renderer.h>

class TestLayer : public toki::Layer {
public:
	TestLayer() = default;

	virtual void on_attach() override;
	virtual void on_detach() override;
	virtual void on_render(toki::RendererCommands& cmd) override;
	virtual void on_update(toki::f32 delta_time) override;

	void submit_test(toki::RendererCommands& cmd);

private:
	toki::Framebuffer m_framebuffer;
	toki::Shader m_shader;
	toki::Buffer m_vertex_buffer;
	toki::Buffer m_index_buffer;
};
