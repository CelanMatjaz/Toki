#pragma once

#include <cstdio>

#if defined(TK_PLATFORM_WINDOWS)
#include "Windows.h"
#endif

namespace toki {

#define TK_ASSERT(condition, message, ...)                    \
    if (condition) {                                          \
    } else {                                                  \
        printf("Assertion failed %s:%i", __FILE__, __LINE__); \
        printf(message __VA_OPT__(, ) __VA_ARGS__);           \
        toki::platform::debug_break();                        \
    }

#if defined(TK_PLATFORM_WINDOWS)
inline void print_error_code() {
    printf("Platform specific error code: %lu", GetLastError());
}
#endif

// Used to print platform specific error code
#define TK_PLATFORM_ASSERT(condition, message, ...)           \
    if (condition) {                                          \
    } else {                                                  \
        printf("Assertion failed %s:%i", __FILE__, __LINE__); \
        printf(message __VA_OPT__(, ) __VA_ARGS__);           \
        print_error_code();                                   \
        toki::platform::debug_break();                        \
    }

}  // namespace toki
