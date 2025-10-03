#pragma once

#include <sys/syscall.h>
#include <toki/core/common/expected.h>
#include <toki/core/string/basic_string.h>
#include <toki/platform/errors.h>
#include <toki/platform/types.h>
#include "toki/core/utils/path.h"

namespace toki::platform {

toki::Expected<void*, PlatformError> allocate(u64 size);

void free(void* ptr);

toki::Expected<u64, PlatformError> write(NativeHandle handle, const void* data, u64 size);

toki::Expected<u64, PlatformError> read(NativeHandle handle, void* data, u64 size);

toki::Expected<toki::Path, PlatformError> getcwd();

}  // namespace toki::platform
