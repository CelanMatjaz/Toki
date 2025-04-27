#pragma once

#include "../core/base.h"
#include "../core/types.h"

namespace toki {

namespace pt {

constexpr u32 PLATFORM_ALLOCATOR_SIZE = MB(16);
constexpr u32 PLATFORM_MAX_WINDOW_COUNT = 2;

enum OpenFlags : i32 {
	OPEN_READ = 0,
	OPEN_WRITE = 1,
	OPEN_RDWR = 2,
};

}  // namespace pt

}  // namespace toki
