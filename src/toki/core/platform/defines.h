#pragma once

#include <toki/core/types.h>

namespace toki {

enum struct FileMode {
	READ,
	WRITE,
	RDWR
};

enum FileFlags : u32 {
	FILE_FLAG_CREATE = 1 << 0,
	FILE_FLAG_OPEN_EXISTING = 1 << 1,
	FILE_FLAG_APPEND = 1 << 2,
	FILE_FLAG_TRUNCATE = 1 << 3,
};

enum struct FileCursorStart {
	BEGIN,
	CURRENT,
	END
};

}  // namespace toki
