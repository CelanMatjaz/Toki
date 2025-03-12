#include "../platform_window.h"

#if defined(TK_PLATFORM_WINDOWS)

#include <Windows.h>

namespace toki {

namespace platform {

#define TK_WIN32_WINDOW_CLASS_NAME "TokiEngineWindowClass"

static HINSTANCE win32_instance{};

void window_system_initialize(const window_system_init& init) {
    WNDCLASSA window_class{};
    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = init.window_proc;
    window_class.hInstance = init.instance;
    window_class.lpszClassName = TK_WIN32_WINDOW_CLASS_NAME;

    TK_PLATFORM_ASSERT(RegisterClassA(&window_class) != 0, "Could not register window class", );

    win32_instance = init.instance;
}

void window_system_shutdown() {}

NativeWindowHandle window_create(const char* title, u32 width, u32 height) {
    HWND handle = CreateWindowA(
        TK_WIN32_WINDOW_CLASS_NAME,
        title,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        0,
        0,
        win32_instance,
        0);

    TK_PLATFORM_ASSERT(handle != 0, "Window was not created");

    return NativeWindowHandle{ .ptr = handle };
}

void window_destroy(NativeWindowHandle handle) {
    DestroyWindow(handle);
}

}  // namespace platform

}  // namespace toki

#endif
