#pragma once

#include <toki/core/core.h>
#include <toki/renderer/commands.h>
#include <toki/renderer/frontend/renderer_frontend.h>

namespace toki {

struct Geometry {
	void* vertices{};
	u32 vertex_data_size{};
	toki::DynamicArray<u32> indices;

	void upload(toki::Renderer* renderer);
	void free(toki::Renderer* renderer);
	void draw(toki::Commands* cmd);

	BufferHandle vertex_buffer{};
	BufferHandle index_buffer{};
};

}  // namespace toki
