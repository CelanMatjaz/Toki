#pragma once

#include "toki/renderer/frontend/renderer_types.h"

namespace toki {

class Commands {
	friend class Renderer;

	Commands(void* data): m_data(data) {}

public:
	Commands() = default;

	virtual void begin_pass(const BeginPassConfig& config);
	virtual void end_pass();

	virtual void bind_shader(ShaderHandle handle);
	virtual void bind_index_buffer(BufferHandle handle);
	virtual void bind_vertex_buffer(BufferHandle handle);
	virtual void bind_uniforms(ShaderLayoutHandle handle);

	virtual void draw(u32 vertex_count);
	virtual void draw_indexed(u32 index_count);

private:
	void* m_data;
};

}  // namespace toki
