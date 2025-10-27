#include "toki/runtime/resources/loaders/texture_loader.h"

#include <toki/runtime/allocators.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace toki {

ResourceData load_texture(const Path& path) {
	ResourceData resource_data{};

	void* pixels = stbi_load(
		path.c_str(),
		&resource_data.metadata.texture.width,
		&resource_data.metadata.texture.height,
		&resource_data.metadata.texture.channels,
		STBI_rgb_alpha);

	resource_data.metadata.texture.channels = 4;

	resource_data.size = resource_data.metadata.texture.width * resource_data.metadata.texture.height *
						 resource_data.metadata.texture.channels;
	resource_data.data = ResourceAllocator::allocate(resource_data.size);

	toki::memcpy(resource_data.data, pixels, resource_data.size);

	stbi_image_free(pixels);

	return resource_data;
}

void unload_texture(ResourceData& resource_data) {
	ResourceAllocator::free(resource_data.data);
	resource_data = {};
}

}  // namespace toki
