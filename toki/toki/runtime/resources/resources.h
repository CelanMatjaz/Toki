#pragma once

#include <toki/core/types.h>

namespace toki {

enum class ResourceType {
	NONE,
	BINARY,
	TEXT
};

struct ResourceData {
	void* data;
	u64 size;
};

}  // namespace toki
