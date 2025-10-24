#pragma once

#include <toki/core/platform/platform_types.h>
#include <toki/core/types.h>

#if defined(TK_PLATFORM_LINUX)

	#include <toki/core/platform/linux/linux.h>

#elif defined(TK_PLATFORM_WINDOWS)

#elif

static_assert(false, "Unsupported platform");

#endif
