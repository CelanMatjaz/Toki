#pragma once

#include "../math/math_types.h"
#include "platform.h"

#if defined(TK_PLATFORM_WINDOWS)
#include "Windows.h"
#endif

namespace toki {

union WindowSystemInit {
#if defined(TK_PLATFORM_WINDOWS)
    HINSTANCE instance;
    LRESULT (*window_proc)(HWND handle, u32 msg, WPARAM w_param, LPARAM l_param);
#endif
};

struct WindowInitFlags {
    b8 show_on_create : 1;
};

void window_system_initialize(const WindowSystemInit& = {});

void window_system_shutdown();

NativeWindowHandle window_create(const char* title, u32 width, u32 height, const WindowInitFlags& flags = {});

void window_destroy(NativeWindowHandle handle);

void window_poll_events();

Vec2<u32> window_get_dimensions(NativeWindowHandle handle);

}  // namespace toki
