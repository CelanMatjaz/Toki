#pragma once

namespace toki::platform {

enum class PlatformError {
	NoError = 0,
	Unknown,
	MemoryAllocationFailed
};

}  // namespace toki
