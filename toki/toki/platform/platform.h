#pragma once

#include <toki/core/types.h>

#if defined(TK_PLATFORM_LINUX)

	#include <toki/platform/linux/linux.h>

#elif defined(TK_PLATFORM_WINDOWS)

#elif

static_assert(false, "Unsupported platform");

#endif

#include <toki/platform/syscalls.h>
#include <toki/platform/window.h>

