#pragma once

#include <toki/platform/types.h>

namespace toki {

void* allocate(u64 size);

void free(void* ptr);

i64 write(toki::NativeHandle handle, const void* data, u64 size);

i64 read(toki::NativeHandle handle, void* data, u64 size);

}  // namespace toki
