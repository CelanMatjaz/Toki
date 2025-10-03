#include <toki/core/common/assert.h>
#include <toki/core/common/expected.h>
#include <toki/core/types.h>
#include <toki/platform/syscalls.h>
#include <unistd.h>

namespace toki::platform {

toki::Expected<u64, PlatformError> write(NativeHandle handle, const void* data, u64 size) {
	i64 result = ::write(handle, data, size);
	if (result == -1) {
		TK_ASSERT(false, "Need to handle error");
		return PlatformError::Unknown;
	}

	return result;
}

toki::Expected<u64, PlatformError> read(NativeHandle handle, void* data, u64 size) {
	i64 result = ::read(handle, data, size);
	if (result == -1) {
		TK_ASSERT(false, "Need to handle error");
		return PlatformError::Unknown;
	}

	return result;
}

}  // namespace toki::platform
