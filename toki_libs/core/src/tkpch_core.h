#pragma once

#include <cstdio>

#if defined(TK_PLATFORM_WINDOWS)
#include "Windows.h"
#endif

#include "core/core.h"

// Memory
#include "memory/allocators/allocator.h"
#include "memory/allocators/bump_allocator.h"
#include "memory/allocators/double_bump_allocator.h"

// Containers
#include "containers/dynamic_array.h"
#include "containers/static_array.h"
// #include "containers/handle_map.h"
// #include "containers/hash_map.h"

// Platform
#include "platform/platform.h"
