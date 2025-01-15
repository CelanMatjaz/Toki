#pragma once

#include "core/base.h"

namespace toki {

namespace platform {

void debug_break();

void* allocate(u32 size);
void deallocate(void* ptr);

}  // namespace platform

}  // namespace toki
