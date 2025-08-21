#pragma once

#include <toki/core/types.h>

namespace toki {

void* allocate(u64 size);

void free(void* ptr);

}  // namespace toki
