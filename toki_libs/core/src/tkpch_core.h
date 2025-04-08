#pragma once

#include <cstdio>

#if defined(TK_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(TK_PLATFORM_LINUX)
#include "platform/linux/linux_platform.h"
#endif

// Core
#include "core/assert.h"
#include "core/base.h"
#include "core/common.h"
#include "core/concepts.h"
#include "core/logging.h"
#include "core/macros.h"
#include "core/string.h"
#include "core/types.h"

// Memory
#include "memory/allocator.h"
#include "memory/bump_allocator.h"
#include "memory/double_bump_allocator.h"

// Containers
#include "containers/dynamic_array.h"
#include "containers/handle_map.h"
#include "containers/hash_map.h"
#include "containers/static_array.h"
