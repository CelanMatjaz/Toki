#pragma once

#include <toki/core/types.h>

namespace toki {

struct Handle {
	u64 value{};

	operator const u64() const {
		return value;
	}

	operator b8() const {
		return value != 0;
	}
};

}  // namespace toki
