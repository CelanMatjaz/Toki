#pragma once

#include "platform.h"

#if defined(TK_PLATFORM_WINDOWS)
#include "Windows.h"
#endif

namespace toki {

namespace platform {

union window_system_init {
#if defined(TK_PLATFORM_WINDOWS)
    HINSTANCE instance;
    LRESULT (*window_proc)(HWND handle, u32 msg, WPARAM w_param, LPARAM l_param);
#endif
};

void window_system_initialize(const window_system_init& = {});

void window_system_shutdown();

NATIVE_HANDLE_TYPE window_create(const char* title, u32 width, u32 height);

void window_destroy(NATIVE_HANDLE_TYPE handle);

}  // namespace platform

}  // namespace toki
