#pragma once

#if defined(TK_PLATFORM_WINDOWS)
	#include "Windows.h"
#elif defined(TK_PLATFORM_LINUX)
#endif

// Core
#include "core/assert.h"
#include "core/base.h"
#include "core/common.h"
#include "core/concepts.h"
#include "core/macros.h"
#include "core/time.h"
#include "core/types.h"
#include "string/string.h"

// Memory
#include "memory/allocator.h"
#include "memory/bump_allocator.h"
#include "memory/double_bump_allocator.h"

// Containers
#include "containers/dynamic_array.h"
#include "containers/handle_map.h"
#include "containers/hash_map.h"
#include "containers/static_array.h"
