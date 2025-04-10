#pragma once

// Core
#include "../../src/core/assert.h"
#include "../../src/core/base.h"
#include "../../src/core/common.h"
#include "../../src/core/logging.h"
#include "../../src/core/string.h"
#include "../../src/types/types.h"
#include "../../src/utils/concepts.h"
#include "../../src/utils/macros.h"

// Allocators
#include "../../src/memory/allocator.h"
#include "../../src/memory/bump_allocator.h"
#include "../../src/memory/double_bump_allocator.h"

// Containers
#include "../../src/containers/basic_ref.h"
#include "../../src/containers/dynamic_array.h"
#include "../../src/containers/handle_map.h"
#include "../../src/containers/hash_map.h"
#include "../../src/containers/static_array.h"

// Platform
#include "../../src/platform/file.h"
#include "../../src/platform/filesystem.h"
#include "../../src/platform/platform.h"
#include "../../src/platform/socket.h"
#include "../../src/platform/stream.h"
#include "../../src/platform/time.h"

#if defined(TK_PLATFORM_LINUX)
#include "../../src/platform/linux/linux_platform.h"
#endif

// Math
#include "../../src/math/math.h"
#include "../../src/math/math_types.h"
