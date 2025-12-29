#include "toki/runtime/resources/loaders/text_loader.h"

#include <toki/core/core.h>
#include <toki/runtime/allocators.h>
#include <toki/runtime/resources/resources.h>

namespace toki {

ResourceData load_text(const Path& path) {
	ResourceData resource_data{};

	auto open_result = toki::open(path.c_str(), FileMode::READ, FileFlags::FILE_FLAG_OPEN_EXISTING);
	if (open_result.is_error()) {
		return {};
	}

	LifetimeWrapper file(toki::move(open_result), [](NativeHandle& handle) {
		auto _ = toki::close(handle);
	});

	auto seek_result = toki::seek(file.get(), 0, FileCursorStart::END);
	if (seek_result.is_error()) {
		return {};
	}

	auto tell_result = toki::tell(file.get());
	if (tell_result.is_error()) {
		return {};
	}
	resource_data.size = tell_result.value();

	resource_data.data = ResourceAllocator::allocate(resource_data.size);
	auto read_result   = toki::read(file.get(), resource_data.data, resource_data.size);
	if (read_result.is_error()) {
		return {};
	}

	return resource_data;
}

void unload_text(ResourceData& resource_data) {
	ResourceAllocator::free(resource_data.data);
	resource_data = {};
}

}  // namespace toki
