#pragma once

#include "core/types.h"

namespace toki {

void* memory_allocate(u64 size);

void memory_free(void* ptr);

}  // namespace toki
