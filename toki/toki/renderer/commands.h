#pragma once

#include "toki/renderer/frontend/renderer_types.h"

namespace toki::renderer {

class Commands {
public:
	virtual void begin_pass() = 0;
	virtual void end_pass() = 0;

	virtual void bind_shader(ShaderHandle handle) = 0;
	virtual void bind_index_buffer(BufferHandle handle) = 0;
	virtual void bind_vertex_buffer(BufferHandle handle) = 0;

	virtual void draw(u32 vertex_count) = 0;
	virtual void draw_indexed(u32 index_count) = 0;
};

}  // namespace toki::renderer
