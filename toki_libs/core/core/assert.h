#pragma once

#include <cstdio>

#include "../platform/platform.h"

#if defined(TK_PLATFORM_WINDOWS)
#include "Windows.h"
#endif

namespace toki {

#define TK_ASSERT(condition, message, ...)                      \
    if (!(condition)) {                                         \
        printf("Assertion failed %s:%i\n", __FILE__, __LINE__); \
        printf(message "\n" __VA_OPT__(, ) __VA_ARGS__);        \
        TK_UNREACHABLE();                                       \
    }

#if defined(TK_PLATFORM_WINDOWS)
#elif defined(TK_PLATFORM_LINUX)
#define TK_ASSERT_PLATFORM_ERROR(value, message, ...)                                                                  \
    if (i64 v = value; v < 0) {                                                                                        \
        printf("Platform error assertion failed with error %i %s:%i\n", -static_cast<i32>(value), __FILE__, __LINE__); \
        printf(message "\n" __VA_OPT__(, ) __VA_ARGS__);                                                               \
        TK_UNREACHABLE();                                                                                              \
    }
#endif

inline void print_error_code() {
#if defined(TK_PLATFORM_WINDOWS)
    printf("Platform specific error code: %lu", GetLastError());
#else

#endif
}

// Used to print platform specific error code
#define TK_PLATFORM_ASSERT(condition, message, ...)           \
    if (condition) {                                          \
    } else {                                                  \
        printf("Assertion failed %s:%i", __FILE__, __LINE__); \
        printf(message __VA_OPT__(, ) __VA_ARGS__);           \
        print_error_code();                                   \
        toki::debug_break();                                  \
    }

}  // namespace toki
