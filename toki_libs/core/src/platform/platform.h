#pragma once

#include "../core/core.h"

#if defined(TK_PLATFORM_WINDOWS)
#include <Windows.h>
#endif

namespace toki {

namespace platform {

using PATH_TYPE = const char*;

union NATIVE_HANDLE_TYPE {
    void* ptr;
    toki::i64 i64;

#if defined(TK_PLATFORM_WINDOWS)
    operator HWND() {
        return reinterpret_cast<HWND>(ptr);
    }
#endif
};

void* memory_allocate(u64 size);

void memory_free(void* ptr);

u64 get_time_microseconds();

u64 get_time_milliseconds();

void debug_break();

void unreachable();

template <typename... Args>
constexpr void print(const char* fmt, Args&&...);

}  // namespace platform

}  // namespace toki
