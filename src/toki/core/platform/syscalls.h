#pragma once

#include <toki/core/common/expected.h>
#include <toki/core/common/optional.h>
#include <toki/core/errors.h>
#include <toki/core/platform/defines.h>
#include <toki/core/platform/platform_types.h>
#include <toki/core/types.h>

namespace toki {

toki::Expected<void*, TokiError> allocate(u64 size);

void free(void* ptr);

toki::Expected<u64, TokiError> write(NativeHandle handle, const void* data, u64 size);

toki::Expected<u64, TokiError> read(NativeHandle handle, void* data, u64 size);

toki::Expected<NativeHandle, TokiError> open(const char* filename, FileMode mode, u32 flags = 0 /* FileFlags */);
toki::Optional<TokiError> close(NativeHandle handle);
toki::Expected<i64, TokiError> set_file_pointer(
	NativeHandle handle, i64 position, FileCursorStart start_from = FileCursorStart::CURRENT);
toki::Expected<u64, TokiError> get_file_pointer(NativeHandle handle);

// Current time in nanoseconds since epoch
toki::u64 get_current_time();

}  // namespace toki
