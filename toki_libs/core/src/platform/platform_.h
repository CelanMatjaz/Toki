#pragma once

#include "../core/types.h"

namespace toki {

namespace platform {

void debug_break();

void* memory_allocate(u64 size);

void memory_free(void* ptr);

}  // namespace platform

}  // namespace toki
