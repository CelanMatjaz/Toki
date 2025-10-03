#pragma once

#include "toki/renderer/frontend/renderer_types.h"

namespace toki::renderer {

class Commands {
public:
	virtual void begin_pass() = 0;
	virtual void end_pass() = 0;

	virtual void bind_shader(const ShaderHandle& handle) = 0;
	virtual void draw(u32 vertex_count) = 0;
};

}  // namespace toki::renderer
