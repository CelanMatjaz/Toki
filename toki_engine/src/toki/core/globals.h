#pragma once

#include "memory/allocators/allocator.h"

#ifndef GLOBAL_ALLOCATOR_SIZE
#define GLOBAL_ALLOCATOR_SIZE Gigabytes(4)
#endif

namespace toki {

inline static Allocator g_allocator(GLOBAL_ALLOCATOR_SIZE);

}
