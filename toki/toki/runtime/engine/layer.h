#pragma once

#include <toki/renderer/renderer.h>

namespace toki::runtime {

class Layer {
	friend class Engine;

public:
	virtual void on_attach() {}
	virtual void on_detach() {}
	virtual void on_update(f32 delta_time) {}
	virtual void on_render(renderer::Commands& commands) {}

protected:
	renderer::Renderer* m_renderer;
};

}  // namespace toki::runtime
