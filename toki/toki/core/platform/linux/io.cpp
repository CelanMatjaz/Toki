#include <toki/core/common/assert.h>
#include <toki/core/common/expected.h>
#include <toki/core/platform/syscalls.h>
#include <toki/core/types.h>
#include <unistd.h>

namespace toki {

toki::Expected<u64, CoreError> write(NativeHandle handle, const void* data, u64 size) {
	i64 result = ::write(static_cast<i32>(handle), data, size);
	if (result == -1) {
		TK_ASSERT(false, "Need to handle error");
		return toki::Unexpected(CoreError::Unknown);
	}

	return static_cast<u64>(result);
}

toki::Expected<u64, CoreError> read(NativeHandle handle, void* data, u64 size) {
	i64 result = ::read(static_cast<i32>(handle), data, size);
	if (result == -1) {
		TK_ASSERT(false, "Need to handle error");
		return toki::Unexpected(CoreError::Unknown);
	}

	return static_cast<u64>(result);
}

}  // namespace toki
