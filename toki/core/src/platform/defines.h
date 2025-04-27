#pragma once

#include <fcntl.h>

#include "types/types.h"

namespace toki {

namespace pt {

enum OpenFlags : i32 {
	OPEN_READ = 0,
	OPEN_WRITE = 1,
	OPEN_RDWR = 2,
};

}  // namespace pt

}  // namespace toki
