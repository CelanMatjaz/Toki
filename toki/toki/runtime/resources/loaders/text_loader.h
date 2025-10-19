#pragma once

#include <toki/core/core.h>
#include <toki/runtime/resources/resources.h>

namespace toki {

template <AllocatorConcept AllocatorType = DefaultAllocator>
ResourceData load_text(const Path& path) {
	ResourceData resource_data{};

	File file(path, FileMode::READ, FileFlags::FILE_FLAG_OPEN_EXISTING);
	file.seek(0, FileCursorStart::END);
	resource_data.size = file.tell();
	file.seek(0, FileCursorStart::BEGIN);

	resource_data.data = AllocatorType::allocate(resource_data.size);
	file.read(resource_data.data, resource_data.size);

	return resource_data;
}

template <AllocatorConcept AllocatorType = DefaultAllocator>
ResourceData unload_text(ResourceData& resource_data) {
	AllocatorType::free(resource_data.data);
	resource_data = {};
}

}  // namespace toki
