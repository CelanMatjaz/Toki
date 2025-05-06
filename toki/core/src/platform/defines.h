#pragma once

#include "../core/base.h"
#include "../core/types.h"

namespace toki {

struct NativeHandle {
#if defined(TK_PLATFORM_LINUX)
	static constexpr i32 INVALID_HANDLE_VALUE = -1;
	NativeHandle(i32 value): handle(static_cast<i64>(value)) {}
	i32 handle;

	operator i32() const {
		return handle;
	}
#endif
	NativeHandle(): handle(INVALID_HANDLE_VALUE) {}
};

struct NativeWindow {
#if defined(TK_WINDOW_SYSTEM_GLFW)
	void* window;
#elif defined(TK_WINDOW_SYSTEM_WAYLAND)
	i32 wl_surface;
#endif
};

constexpr u32 PLATFORM_ALLOCATOR_SIZE = MB(16);
constexpr u32 PLATFORM_MAX_WINDOW_COUNT = 2;

enum FileOpenFlags : u32 {
	FILE_READ = 1,
	FILE_WRITE = 2,
	FILE_RDWR = FILE_READ | FILE_WRITE,
	FILE_TRUNC,
	FILE_APPEND,
};

}  // namespace toki
