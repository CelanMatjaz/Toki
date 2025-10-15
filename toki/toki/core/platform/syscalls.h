#pragma once

#include <toki/core/common/expected.h>
#include <toki/core/errors.h>
#include <toki/core/platform/platform_types.h>
#include <toki/core/types.h>

namespace toki {

toki::Expected<void*, CoreError> allocate(u64 size);

void free(void* ptr);

toki::Expected<u64, CoreError> write(NativeHandle handle, const void* data, u64 size);

toki::Expected<u64, CoreError> read(NativeHandle handle, void* data, u64 size);

// Current time in nanoseconds since epoch
toki::u64 get_current_time();

}  // namespace toki
