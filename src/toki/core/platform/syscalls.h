#pragma once

#include <toki/core/common/defines.h>
#include <toki/core/common/expected.h>
#include <toki/core/common/optional.h>
#include <toki/core/errors.h>
#include <toki/core/platform/defines.h>
#include <toki/core/platform/platform_types.h>
#include <toki/core/types.h>

namespace toki {

TK_NODISCARD toki::Expected<void*, TokiError> allocate(u64 size);

TK_NODISCARD TokiError free(void* ptr);

TK_NODISCARD toki::Expected<u64, TokiError> write(NativeHandle handle, const void* data, u64 size);

TK_NODISCARD toki::Expected<u64, TokiError> read(NativeHandle handle, void* data, u64 size);

TK_NODISCARD toki::Expected<NativeHandle, TokiError> open(const char* filename, FileMode mode, u32 flags);

TK_NODISCARD TokiError close(NativeHandle handle);

TK_NODISCARD toki::Expected<i64, TokiError> seek(NativeHandle handle, i64 position, FileCursorStart start_from);

TK_NODISCARD toki::Expected<u64, TokiError> tell(NativeHandle handle);

TK_NODISCARD toki::Expected<u64, TokiError> get_current_time();

TK_NODISCARD TokiError sleep(u32 millis);

}  // namespace toki
