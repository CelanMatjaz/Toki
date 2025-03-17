#pragma once

#include "../core/core.h"

#if defined(TK_PLATFORM_WINDOWS)
#include <Windows.h>
#endif

namespace toki {

using PATH_TYPE = const char*;

union NativeWindowHandle {
    void* ptr;
    toki::i64 i64;

    inline operator u64() {
        return i64;
    }

#if defined(TK_PLATFORM_WINDOWS)
    inline operator HWND() {
        return reinterpret_cast<HWND>(ptr);
    }
#endif
};

void* memory_allocate(u64 size);

void memory_free(void* ptr);

u64 time_microseconds();

u64 time_milliseconds();

void debug_break();

void unreachable();

template <typename... Args>
constexpr void print(const char* fmt, Args&&...);

}  // namespace toki
