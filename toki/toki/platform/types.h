#pragma once

#include <toki/core/types.h>

namespace toki::platform {

struct NativeHandle {
#if defined(TK_PLATFORM_LINUX)
	static constexpr i64 INVALID_HANDLE_VALUE = -1;
	NativeHandle(i64 value): handle(value) {}
	i64 handle;

	operator i64() const {
		return handle;
	}
#endif
	NativeHandle(): handle(INVALID_HANDLE_VALUE) {}
};

}  // namespace toki
