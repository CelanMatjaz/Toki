#pragma once

#include <toki/core/types.h>

namespace toki {

enum class ResourceType {
	NONE,
	BINARY,
	TEXT,
	TEXTURE
};

union ResourceMetadata {
	struct {
		i32 width, height, channels;
	} texture;
	struct {
	} model;
	struct {
	} binary, text;
};

struct ResourceData {
	void* data;
	u64 size;
	ResourceMetadata metadata;
};

}  // namespace toki
