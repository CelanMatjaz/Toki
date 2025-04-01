#pragma once

#include "../core/core.h"

#if defined(TK_PLATFORM_WINDOWS)
#include <Windows.h>

#elif defined(TK_PLATFORM_LINUX) && defined(TK_WINDOW_SYSTEM_WAYLAND)
struct wl_display;
struct wl_surface;
#endif

namespace toki {

#if (defined(__GNUC__) || defined(__clang__)) && __cplusplus >= 202302L
#define UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
#define UNREACHABLE _STL_UNREACHABLE;
#else
#error "unreachable compiler function not found, only Clang, GCC and MSVC supported"
#endif

using PATH_TYPE = const char*;

union NativeWindowHandle {
    void* ptr;
    toki::i64 i64;

    inline operator u64() {
        return i64;
    }

#if defined(TK_PLATFORM_WINDOWS) && defined(TK_WINDOW_SYSTEM_WINDOWS)
    inline operator HWND() {
        return reinterpret_cast<HWND>(ptr);
    }
#elif defined(TK_PLATFORM_LINUX) && defined(TK_WINDOW_SYSTEM_WAYLAND)
    struct {
        uint32_t wl_display, wl_surface;
    } double_u32;

    struct wl_display* wl_display() {
        return reinterpret_cast<struct wl_display*>(&double_u32.wl_display);
    }

    struct wl_surface* wl_surface() {
        return reinterpret_cast<struct wl_surface*>(&double_u32.wl_surface);
    }
#endif
};

void* memory_allocate(u64 size);

void memory_free(void* ptr);

u64 time_microseconds();

u64 time_milliseconds();

void debug_break();

}  // namespace toki
