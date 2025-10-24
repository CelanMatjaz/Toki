#include "toki/runtime/resources/loaders/text_loader.h"

#include <toki/core/core.h>
#include <toki/runtime/allocators.h>
#include <toki/runtime/resources/resources.h>

namespace toki {

ResourceData load_text(const Path& path) {
	ResourceData resource_data{};

	File file(path, FileMode::READ, FileFlags::FILE_FLAG_OPEN_EXISTING);
	file.seek(0, FileCursorStart::END);
	resource_data.size = file.tell();
	file.seek(0, FileCursorStart::BEGIN);

	resource_data.data = ResourceAllocator::allocate(resource_data.size);
	file.read(resource_data.data, resource_data.size);

	return resource_data;
}

void unload_text(ResourceData& resource_data) {
	ResourceAllocator::free(resource_data.data);
	resource_data = {};
}

}  // namespace toki
