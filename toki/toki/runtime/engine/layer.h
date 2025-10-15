#pragma once

#include <toki/renderer/renderer.h>

namespace toki::runtime {

class Layer {
	friend class Engine;

public:
	virtual void on_attach() {}
	virtual void on_detach() {}
	virtual void on_update([[maybe_unused]] f32 delta_time) {}
	virtual void on_render([[maybe_unused]] renderer::Commands* commands) {}
	virtual void on_event([[maybe_unused]] Window* window, [[maybe_unused]] Event& event) {}

protected:
	renderer::Renderer* m_renderer;
};

}  // namespace toki::runtime
