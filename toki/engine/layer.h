#pragma once

#include "toki/renderer.h"
#include "toki/renderer_commands.h"

namespace toki {

class Layer {
	friend class Engine;

public:
	Layer() = default;

	virtual void on_attach() = 0;
	virtual void on_detach() = 0;
	virtual void on_render(RendererCommands& cmd) = 0;
	virtual void on_update(f32 delta_time) = 0;

protected:
	Layer(RendererFrontend* renderer): m_renderer(renderer) {}
	RendererFrontend* m_renderer;
};

}  // namespace toki
