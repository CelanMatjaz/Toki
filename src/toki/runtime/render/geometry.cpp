#include <toki/runtime/render/geometry.h>

namespace toki {

void Geometry::upload(toki::Renderer* renderer) {
	{
		BufferConfig buffer_config{};
		buffer_config.type = BufferType::VERTEX;
		buffer_config.size = vertex_data_size;

		vertex_buffer = renderer->create_buffer(buffer_config);
		renderer->set_buffer_data(vertex_buffer, vertices, vertex_data_size);
	}

	{
		BufferConfig buffer_config{};
		buffer_config.type = BufferType::INDEX;
		buffer_config.size = indices.size() * sizeof(u32);

		index_buffer = renderer->create_buffer(buffer_config);
		renderer->set_buffer_data(index_buffer, indices.data(), buffer_config.size);
	}
}

void Geometry::free(toki::Renderer* renderer) {
	renderer->destroy_handle(vertex_buffer);
	renderer->destroy_handle(index_buffer);
}

void Geometry::draw(toki::Commands* cmd) {
	cmd->bind_index_buffer(index_buffer);
	cmd->bind_vertex_buffer(vertex_buffer);
	cmd->draw_indexed(indices.size());
}

}  // namespace toki
